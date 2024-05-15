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
	const float h = getWidth() * 0.5f;
	const float k = getHeight() * 0.5f;

	// In local coordinates, the radius is equal to the center of the bounding rectangle
	// Temporarily added a multiplier
	const float a = h;// *0.5f;
	const float b = k;// *0.5f;

	return (x - h) * (x - h) / (a * a) + (y - k) * (y - k) / (b * b) <= 1.f;
}
