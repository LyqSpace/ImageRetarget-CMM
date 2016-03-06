#pragma once

/************************************************************************/
/* Draw mesh using matlab functions and save image if necessary.        */
/* Input:																*/
/*		points: m*n*2 matrix representing nodes of mesh					*/
/*		saveName: name of image without extension.						*/
/************************************************************************/
void DrawMesh(const double* pointsX, const double* pointsY, int m, int n, const char* saveName = NULL);