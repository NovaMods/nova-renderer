package com.continuum.nova.utils;

import java.awt.*;
import java.awt.geom.Point2D;
import java.nio.Buffer;
import java.util.*;
import java.io.*;
import javax.imageio.*;
import java.awt.image.*;
import java.util.List;

/**
 * From https://github.com/lukaszdk/texture-atlas-generator
 *
 * <p>Modified significantly to work for me</p>
 *
 * @author lukaszdk
 * @author ddubois
 */

public class AtlasGenerator {
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

        count = 0;

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

        private class LargestComparator implements Comparator {

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

        private class SmallestComparator implements Comparator {

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

        private class Node {
            public Rectangle rect;
            public Node child[];
            public BufferedImage image;

            public Node(int x, int y, int width, int height) {
                rect = new Rectangle(x, y, width, height);
                child = new Node[2];
                child[0] = null;
                child[1] = null;
                image = null;
            }

            public boolean IsLeaf() {
                return child[0] == null && child[1] == null;
            }

            // Algorithm from http://www.blackpawn.com/texts/lightmaps/
            public Node Insert(BufferedImage image, int padding) {
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
                }

                if(child[1] != null) {
                    Point2D child2Point = child[1].getComparedPoint(comparePoint, comparator);

                    minX = comparator.getComparedValue(minX, child2Point.getX());
                    minY = comparator.getComparedValue(minY, child2Point.getY());
                }

                minX = comparator.getComparedValue(minX, comparator.getXValue(rect));
                minY = comparator.getComparedValue(minY, comparator.getYValue(rect));

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
        public BufferedImage getImage() {
            Point2D smallestPos = getAtlasMinBounds();
            Point2D biggestPos = getAtlasMaxBounds();

            BufferedImage img = image.getSubimage((int)smallestPos.getX(), (int)smallestPos.getY(), (int)biggestPos.getX(), (int)biggestPos.getY());
            BufferedImage copyOfImage = new BufferedImage(img.getWidth(), img.getHeight(), BufferedImage.TYPE_4BYTE_ABGR);
            Graphics g = copyOfImage.createGraphics();
            g.drawImage(img, 0, 0, null);
            return copyOfImage;
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
            return root.getComparedPoint(new Point2D.Double(100000, 100000), new SmallestComparator());
        }

        /**
         * Returns the maximum X and Y of the pixels used by the atlas
         * @return
         */
        private Point2D getAtlasMaxBounds() {
            return root.getComparedPoint(new Point2D.Double(-100, -100), new LargestComparator());
        }
    }
}
