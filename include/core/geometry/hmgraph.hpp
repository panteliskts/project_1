#pragma once

#include <cmath>
#include <vector>

#include <Eigen/Dense>


using Point2 = Eigen::Vector2d;

inline double dist(const Point2& a, const Point2& b) { return std::hypot(a.x() - b.x(), a.y() - b.y()); }

bool fitHomography(const std::vector<Point2>& src, const std::vector<Point2>& dst, Eigen::Matrix3d& H);


bool project(const Eigen::Matrix3d& H, const Point2& p, Point2& out);


bool isValidHomography(const Eigen::Matrix3d& H);

double cornerError(const Eigen::Matrix3d& H, const Eigen::Matrix3d& Hgt, int width, int height);
