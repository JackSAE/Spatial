#include "CImg.h"
#include "kf/kf_vector.h"
#include "kf/kf_ray.h"
#include "windows.h"
#include "vector"
#include "iostream"
#include <map>

using namespace cimg_library;

// The resolution of the window and the output of the ray tracer. This can be overridden by the Lua startup script.
int g_windowWidth = 1024;
int g_windowHeight = 1024;

class Circle
{
public:
	kf::Vector2 m_pos;
	float m_radius;

	kf::Vector2 c_left;
	kf::Vector2 c_right;
};

class Grid
{
public:
	kf::Vector2 m_pos;

	kf::Vector2 g_left;
	kf::Vector2 g_right;

	std::vector<Circle> g_circles;
};

std::map<std::pair<int, int>, Grid> grids;




//Main Loop
int main(int argc, char **argv)
{
	srand(0);

	// The floating point image target that the scene is rendered into.
	CImg<float> image(g_windowWidth, g_windowHeight, 1, 3, 0);

	// The display object used to show the image.
	CImgDisplay main_disp(image, "Spatial");

	main_disp.set_normalization(0);	// Normalisation 0 disables auto normalisation of the image (scales to make the darkest to brightest colour fit 0 to 1 range.


	//Sets up the Grid drawing lines
	int GridDivisionNumber = 1; //Number of squares across
	kf::Vector2 gridCentre = kf::Vector2(g_windowWidth / GridDivisionNumber, g_windowHeight / GridDivisionNumber);
	for (int i = 0; i < GridDivisionNumber; ++i)
	{
		for (int j = 0; j < GridDivisionNumber; ++j)
		{
			grids[std::make_pair(j, i)].g_left = kf::Vector2(gridCentre.x * j , gridCentre.y * i);
			grids[std::make_pair(j, i)].g_right = kf::Vector2(gridCentre.x * (j + 1) , gridCentre.y * (i + 1) );
		}
	}


	//Looping through and setting circles
	for (int i = 0; i < 1000; ++i)
	{
		Circle c;
		c.m_pos.x = rand() % g_windowWidth;
		c.m_pos.y = rand() % g_windowHeight;
		c.m_radius = rand() % 50;

		c.c_left = kf::Vector2(c.m_pos.x - c.m_radius, c.m_pos.y - c.m_radius);
		c.c_right = kf::Vector2(c.m_pos.x + c.m_radius, c.m_pos.y + c.m_radius);

		//Get an iterator of grids and loop through and add all circles into the grid.
		typedef std::map<std::pair<int, int>, Grid>::iterator it;
		for (it i = grids.begin(); i != grids.end(); ++i)
		{
			i->second.g_circles.push_back(c);
		}
	}


	// Record the starting time.
	DWORD startTime = timeGetTime();

	//Main Loop for drawing circles
	typedef std::map<std::pair<int, int>, Grid>::iterator it;
	for (it i = grids.begin(); i != grids.end(); ++i)
	{
		for (int y = i->second.g_left.x; y < i->second.g_right.y; ++y)
		{
			for (int x = i->second.g_left.x; x < i->second.g_right.y; ++x)
			{
				kf::Colour output(0, 0, 0, 0);
				float e = 0;
				kf::Vector2 c_Pos = kf::Vector2(x, y);

				for (Circle c : i->second.g_circles)
				{
					if ((c_Pos - c.m_pos).lengthSquared() <= c.m_radius * c.m_radius)
					{
						float d = (c_Pos - c.m_pos).length();
						e += (1.0 - d / c.m_radius);
					}
				}

				output.set(e, e, e, 1);

				// Clamp the output colour to 0-1 range before conversion.
				output.saturate();

				// Convert from linear space to sRGB.
				output.toSRGB();

				// Write the colour to the image (scaling up by 255).
				*image.data(x, y, 0, 0) = output.r * 255;
				*image.data(x, y, 0, 1) = output.g * 255;
				*image.data(x, y, 0, 2) = output.b * 255;
			}
			// Check for Escape key.
			if (main_disp.is_keyESC())
				return 0;
		}
		main_disp.display(image); // If thiis is enabled you can see it live.
	}

	// Record ending time.
	DWORD endTime = timeGetTime();

	// Display elapsed time in the window title bar.
	main_disp.set_title("Render time: %dms", endTime - startTime);
	main_disp.display(image);

	// Keep refreshing the window until it is closed or escape is hit.
	while (!main_disp.is_closed())
	{
		if (main_disp.is_keyESC())
			return 0;
		main_disp.wait();
	}

	return 0;

}

///<Summary>
///Old Code used to load the circles
///<Summary>

// Primary loop through all screen pixels.
/*
for (int y = 0; y < g_windowHeight; ++y)
{
for (int x = 0; x < g_windowWidth; ++x)
{
kf::Colour output(0,0,0,0);
float e = 0;
for (int i = 0; i < g_circles.size(); ++i)
{
float d = (kf::Vector2(x, y) - g_circles[i].m_pos).length();
if (d < g_circles[i].m_radius)
{
e = e + (1.0-d / g_circles[i].m_radius);
}
}
output.set(e, e, e, 1);

// Clamp the output colour to 0-1 range before conversion.
output.saturate();

// Convert from linear space to sRGB.
output.toSRGB();

// Write the colour to the image (scaling up by 255).
*image.data(x, y, 0, 0) = output.r*255;
*image.data(x, y, 0, 1) = output.g*255;
*image.data(x, y, 0, 2) = output.b*255;
}
std::cout << y << std::endl;
// Check for Escape key.
if (main_disp.is_keyESC())
return 0;
}
*/

