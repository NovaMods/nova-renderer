package com.continuum.nova.utils;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * From https://github.com/lukaszdk/texture-atlas-generator
 *
 * <p>Modified significantly to work for me</p>
 *
 * @author lukaszdk
 * @author ddubois
 */
public class AtlasGenerator {
    private static final Logger LOG = LogManager.getLogger(AtlasGenerator.class);

    public List<Texture> Run(String name, int width, int height, int padding, boolean unitCoordinates, List<ImageName> images) {

        List<Texture> textures = new ArrayList<>();

        textures.add(new Texture(width, height));

        int count = 0;

        for(ImageName imageName : images) {
            boolean added = false;

            System.out.println("Adding " + imageName.name + " to atlas (" + (++count) + ")");

            for(Texture texture : textures) {
                if(texture.AddImage(imageName.image, imageName.name, padding)) {
                    added = true;
                    break;
                }
            }

            if(!added) {
                Texture texture = new Texture(width, height);
                texture.AddImage(imageName.image, imageName.name, padding);
                textures.add(texture);
            }
        }

        return textures;
    }

    public static class ImageName {
        public BufferedImage image;
        public String name;

        public ImageName(BufferedImage image, String name) {
            this.image = image;
            this.name = name;
        }
    }

    public static class Texture {
        private interface Comparator {
            double getComparedValue(double val1, double val2);

            double getXValue(Rectangle rect);
            double getYValue(Rectangle rect);
        }

        private class Largest implements Comparator {

            @Override
            public double getComparedValue(double val1, double val2) {
                return val1 > val2 ? val1 : val2;
            }

            @Override
            public double getXValue(Rectangle rect) {
                return rect.getWidth();
            }

            @Override
            public double getYValue(Rectangle rect) {
                return rect.getHeight();
            }
        }

        private class Smallest implements Comparator {

            @Override
            public double getComparedValue(double val1, double val2) {
                return val1 < val2 ? val1 : val2;
            }

            @Override
            public double getXValue(Rectangle rect) {
                return rect.getX();
            }

            @Override
            public double getYValue(Rectangle rect) {
                return rect.getY();
            }
        }

        public static class WrongNumComponentsException extends Exception {
            WrongNumComponentsException(String msg) {
                super(msg);
            }
        }

        private class Node {
            Rectangle rect;
            public Node child[];
            public BufferedImage image;

            Node(int x, int y, int width, int height) {
                rect = new Rectangle(x, y, width, height);
                child = new Node[2];
                child[0] = null;
                child[1] = null;
                image = null;
            }

            boolean IsLeaf() {
                return child[0] == null && child[1] == null;
            }

            // Algorithm from http://www.blackpawn.com/texts/lightmaps/
            Node Insert(BufferedImage image, int padding) {
                if(!IsLeaf()) {
                    Node newNode = child[0].Insert(image, padding);

                    if(newNode != null) {
                        return newNode;
                    }

                    return child[1].Insert(image, padding);
                } else {
                    if(this.image != null) {
                        return null; // occupied
                    }

                    if(image.getWidth() > rect.width || image.getHeight() > rect.height) {
                        return null; // does not fit
                    }

                    if(image.getWidth() == rect.width && image.getHeight() == rect.height) {
                        this.image = image; // perfect fit
                        return this;
                    }

                    int dw = rect.width - image.getWidth();
                    int dh = rect.height - image.getHeight();

                    if(dw > dh) {
                        child[0] = new Node(rect.x, rect.y, image.getWidth(), rect.height);
                        child[1] = new Node(padding + rect.x + image.getWidth(), rect.y, rect.width - image.getWidth() - padding, rect.height);
                    } else {
                        child[0] = new Node(rect.x, rect.y, rect.width, image.getHeight());
                        child[1] = new Node(rect.x, padding + rect.y + image.getHeight(), rect.width, rect.height - image.getHeight() - padding);
                    }
					/*if(dw > dh)
					{
						child[0] = new Node(rect.x, rect.y, image.getWidth(), rect.height);
						child[1] = new Node(padding+rect.x+image.getWidth(), rect.y, rect.width - image.getWidth(), rect.height);
					}
					else
					{
						child[0] = new Node(rect.x, rect.y, rect.width, image.getHeight());
						child[1] = new Node(rect.x, padding+rect.y+image.getHeight(), rect.width, rect.height - image.getHeight());
					}*/

                    return child[0].Insert(image, padding);
                }
            }

            Point2D getComparedPoint(Point2D comparePoint, Comparator comparator) {
                double minX = comparePoint.getX();
                double minY = comparePoint.getY();

                if(child[0] != null) {
                    Point2D child1Point = child[0].getComparedPoint(comparePoint, comparator);

                    minX = comparator.getComparedValue(minX, child1Point.getX());
                    minY = comparator.getComparedValue(minY, child1Point.getY());

                    LOG.info(comparator.getClass().getSimpleName() + " point after looking at child 1: " + minX + "," + minY);
                }

                if(child[1] != null) {
                    Point2D child2Point = child[1].getComparedPoint(comparePoint, comparator);

                    minX = comparator.getComparedValue(minX, child2Point.getX());
                    minY = comparator.getComparedValue(minY, child2Point.getY());

                    LOG.info(comparator.getClass().getSimpleName() + " point after looking at child 2: " + minX + "," + minY);
                }

                if(image != null) {
                    minX = comparator.getComparedValue(minX, comparator.getXValue(rect));
                    minY = comparator.getComparedValue(minY, comparator.getYValue(rect));
                }

                LOG.info(comparator.getClass().getSimpleName() + " point after looking at self: " + minX + "," + minY);

                return new Point2D.Double(minX, minY);
            }
        }

        private BufferedImage image;
        private Graphics2D graphics;
        private Node root;
        private Map<String, Rectangle> rectangleMap;

        public Texture(int width, int height) {
            image = new BufferedImage(width, height, BufferedImage.TYPE_4BYTE_ABGR);
            graphics = image.createGraphics();

            root = new Node(0, 0, width, height);
            rectangleMap = new TreeMap<>();
        }

        boolean AddImage(BufferedImage image, String name, int padding) {
            Node node = root.Insert(image, padding);

            if(node == null) {
                return false;
            }

            rectangleMap.put(name, node.rect);
            graphics.drawImage(image, null, node.rect.x, node.rect.y);


            return true;
        }

        /**
         * Gets the subarea of the image that's actually used by the atlas
         *
         * <p>So like, if the image is 16K by 16K and the atlas only uses up a 2K by 1K area, the returned image will
         * be 2K by 1K</p>
         *
         * <p>The original image is preserved, because reasons. I may make it not preserved, though</p>
         *
         * @return An image which covers the smallest possible area
         */
        public BufferedImage getImage() throws WrongNumComponentsException {
            Point2D smallestPos = getAtlasMinBounds();
            Point2D biggestPos = getAtlasMaxBounds();

            byte[] imageData = ((DataBufferByte)image.getRaster().getDataBuffer()).getData();

            /*
            Loop through the image's pixels, grabbing the ones we care about
             */
            int imageWidth = (int) (biggestPos.getX() - smallestPos.getX());
            int imageHeight = (int) (biggestPos.getY() - smallestPos.getY());
            int numComponents = image.getColorModel().getNumComponents();

            if(numComponents != 3 && numComponents != 4) {
                throw new WrongNumComponentsException("Texture should have three or four components, but it has " + numComponents);
            }

            byte[] newData = new byte[imageWidth * imageHeight * numComponents];

            // I'm manually copying the data over because this works, and the Graphics method, although much cleaner and
            // simpler, does not work.
            for(int y = 0; y < imageHeight; y++) {
                System.arraycopy(imageData, (int) (smallestPos.getX() + y * image.getWidth() * numComponents), newData, y * imageWidth * numComponents, numComponents * imageWidth);
            }

            BufferedImage newImage = new BufferedImage(imageWidth, imageHeight, numComponents == 4 ? BufferedImage.TYPE_4BYTE_ABGR : BufferedImage.TYPE_3BYTE_BGR);
            byte[] dst = ((DataBufferByte)newImage.getRaster().getDataBuffer()).getData();
            System.arraycopy(newData, 0, dst, 0, newData.length);

            image = newImage;

            try {
                File imageFile = new File("Image.jpg");
                ImageIO.write(image, "jpg", imageFile);
            } catch(IOException e) {
                e.printStackTrace();
            }

            return newImage;
        }

        public Map<String, Rectangle> getRectangleMap() {
            return rectangleMap;
        }

        /**
         * Calculates the minimum X and Y values of the pixels used by the texture atlas. In theory this will always be
         * zero, but I don't trust theory
         *
         * @return The minimum X and Y used by the atlas
         */
        private Point2D getAtlasMinBounds() {
            return root.getComparedPoint(new Point2D.Double(100000, 100000), new Smallest());
        }

        /**
         * Returns the maximum X and Y of the pixels used by the atlas
         *
         * @return The maximum X and Y of the pixels used by the atlas
         */
        private Point2D getAtlasMaxBounds() {
            return root.getComparedPoint(new Point2D.Double(0, 0), new Largest());
        }
    }
}
