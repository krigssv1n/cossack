/*
  ==============================================================================

    EllipticSlider.cpp
    Created: 22 Apr 2024 5:50:48am
    Author:  KOT

  ==============================================================================
*/

#include "EllipticSlider.h"

EllipticSlider::EllipticSlider()
{
}

bool EllipticSlider::hitTest(int x, int y)
{
	// Handle setInterceptsMouseClicks()
	if (!Component::hitTest(x, y))
		return false;

	// Check if the point is inside the ellipse
	// TODO: Rotation is not handled
	
	// General formula is:
	// (x - h) ^ 2 / a ^ 2 + (y - k) ^ 2 / b ^ 2 <= 1

	// Center
	const int h = getWidth() / 2;
	const int k = getHeight() / 2;

	// In local coordinates, the radius is equal to the center of the bounding rectangle
	// Temporarily added a multiplier
	const int a = h * 0.5;
	const int b = k * 0.5;

	return (x - h) * (x - h) / (a * a) + (y - k) * (y - k) / (b * b) <= 1.f;
}
