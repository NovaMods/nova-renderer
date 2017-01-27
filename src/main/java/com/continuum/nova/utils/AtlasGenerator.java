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

public class AtlasGenerator {
    private static final Logger LOG = LogManager.getLogger(AtlasGenerator.class);

    public List<Texture> Run(int width, int height, int padding, List<ImageName> images) {
        List<Texture> textures = new ArrayList<>();
        textures.add(new Texture(width, height));

        for(ImageName imageName : images) {
            boolean added = false;

            // Write the image so we can verify we're reading it in correctly
            // From this code, I know that the data in the image is loaded, although it looks like the red and blue
            // channels are swapped
            /*try {
                String[] splitPath = imageName.name.split("/");
                File imageFile = new File(splitPath[splitPath.length - 1]);
                ImageIO.write(imageName.image, "png", imageFile);
            } catch(IOException e) {
                LOG.warn("Could not write image " + imageName, e);
            }*/

            for(Texture texture : textures) {
                if(texture.addImage(imageName.image, imageName.name, padding)) {
                    added = true;
                    break;
                }
            }

            if(!added) {
                Texture texture = new Texture(width, height);
                texture.addImage(imageName.image, imageName.name, padding);
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

            boolean isLeaf() {
                return child[0] == null && child[1] == null;
            }

            Node insert(BufferedImage image, int padding) {
                LOG.debug("Inserting an image with padding {}", padding);
                if(!isLeaf()) {
                    Node newNode = child[0].insert(image, padding);

                    if(newNode != null) {
                        return newNode;
                    }

                    return child[1].insert(image, padding);
                } else {
                    if(this.image != null) {
                        return null;
                    }

                    if(image.getWidth() > rect.width || image.getHeight() > rect.height) {
                        return null;
                    }

                    if(image.getWidth() == rect.width && image.getHeight() == rect.height) {
                        this.image = image;
                        return this;
                    }

                    int dw = rect.width - image.getWidth();
                    int dh = rect.height - image.getHeight();

                    // Prefer filling vertically before horizontally. MC has a number of textures that are much higher
                    // than they are wide
                    // TODO: Figure out how to make the atlas texture as square as possible
                    if(dh > dw) {
                        child[0] = new Node(rect.x, rect.y, rect.width, image.getHeight());
                        child[1] = new Node(rect.x, padding + rect.y + image.getHeight(), rect.width, rect.height - image.getHeight() - padding);
                    } else {
                        child[0] = new Node(rect.x, rect.y, image.getWidth(), rect.height);
                        child[1] = new Node(padding + rect.x + image.getWidth(), rect.y, rect.width - image.getWidth() - padding, rect.height);
                    }

                    return child[0].insert(image, padding);
                }
            }

            /**
             * Figures out the maximum bounds of this node or any of its children
             *
             * @param comparePoint The point to compare the bounds of this nide and its children to
             * @return The maximum bounds of this node or any of its children
             */
            Point2D getMaxBounds(Point2D comparePoint) {
                // yes, there's duplicate logic. This code is simple. Bite me.
                double maxX = comparePoint.getX();
                double maxY = comparePoint.getY();

                if(child[0] != null) {
                    Point2D childPoint = child[0].getMaxBounds(comparePoint);
                    if(childPoint.getX() > maxX) {
                        maxX = childPoint.getX();
                    }
                    if(childPoint.getY() > maxY) {
                        maxY = childPoint.getY();
                    }
                }

                if(child[1] != null) {
                    Point2D childPoint = child[1].getMaxBounds(comparePoint);
                    if(childPoint.getX() > maxX) {
                        maxX = childPoint.getX();
                    }
                    if(childPoint.getY() > maxY) {
                        maxY = childPoint.getY();
                    }
                }

                if(rect.getX() > maxX) {
                    maxX = rect.getX();
                }
                if(rect.getY() > maxY) {
                    maxY = rect.getY();
                }

                return new Point2D.Double(maxX, maxY);
            }

            void drawAllImages(Graphics2D graphics) {
                LOG.trace("Drawing image at position " + rect.x + ", " + rect.y);
                graphics.drawImage(image, null, rect.x, rect.y);
                if(child[0] != null) {
                    child[0].drawAllImages(graphics);
                }
                if(child[1] != null) {
                    child[1].drawAllImages(graphics);
                }
            }
        }

        private Node root;
        private Map<String, Rectangle> rectangleMap;

        public Texture(int width, int height) {
            root = new Node(0, 0, width, height);
            rectangleMap = new TreeMap<>();
        }

        boolean addImage(BufferedImage image, String name, int padding) {
            Node node = root.insert(image, padding);

            if(node == null) {
                return false;
            }

            rectangleMap.put(name, node.rect);
            return true;
        }

        public BufferedImage getImage() throws WrongNumComponentsException {
            Point2D biggestPos = root.getMaxBounds(new Point2D.Double(0, 0));
            int imageWidth = (int) biggestPos.getX();
            int imageHeight = (int) biggestPos.getY();

            LOG.info("Image bounds: " + biggestPos);
            LOG.info("Making an image of size " + imageWidth + ", " + imageHeight);
            BufferedImage image = new BufferedImage(imageWidth, imageHeight, BufferedImage.TYPE_4BYTE_ABGR);
            int numComponents = image.getColorModel().getNumComponents();

            if(numComponents != 3 && numComponents != 4) {
                throw new WrongNumComponentsException("Texture should have three or four components, but it has " + numComponents);
            }

            renderAtlas(image);

            byte[] imageData = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();
            byte[] newData = new byte[imageWidth * imageHeight * numComponents];

            for(int y = 0; y < imageHeight; y++) {
                System.arraycopy(imageData, y * image.getWidth() * numComponents, newData, y * imageWidth * numComponents, numComponents * imageWidth);
            }

            BufferedImage newImage = new BufferedImage(imageWidth, imageHeight, numComponents == 4 ? BufferedImage.TYPE_4BYTE_ABGR : BufferedImage.TYPE_3BYTE_BGR);
            byte[] dst = ((DataBufferByte) newImage.getRaster().getDataBuffer()).getData();
            System.arraycopy(newData, 0, dst, 0, newData.length);
            //image = newImage;

            try {
                // When I print out the image here, the image is completely black
                // This implies that I'm not properly drawing the incoming images on the texture atlas
                File imageFile = new File("Image.jpg");
                ImageIO.write(image, "jpg", imageFile);
            } catch(IOException e) {
                e.printStackTrace();
            }

            return image;
        }

        /**
         * Renders the atlas into the provided image
         *
         * @param image The Image to render the atlas into
         */
        private void renderAtlas(BufferedImage image) {
            Graphics2D graphics = image.createGraphics();

            root.drawAllImages(graphics);
        }

        public Map<String, Rectangle> getRectangleMap() {
            return rectangleMap;
        }
    }
}
