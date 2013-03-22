/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test whether resizing a texture and switching from/to mipmapping
 * works correctly.
 */

#include <stdarg.h>

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 128;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const int CellSize = 8; /* see cell_coords */

static int testnr;

static void make_color(int nr, float* result)
{
	int i;

	for(i = 0; i < 4; ++i) {
		result[i] = ((nr & 3) + 1) * 0.2;
		nr >>= 2;
	}
}

static void cell_coords(int cellnr, int *px, int *py)
{
	*px = CellSize * (cellnr & 15); /* Hard-coded things! */
	*py = CellSize * (cellnr / 16);
	assert(cellnr < 256);
}

/** Load a texture image with a constant color generated by \ref make_color */
static void teximage2d(GLuint lvl, GLuint w, GLuint h, int colornum)
{
	float color[4];
	float* tex;
	int i;

	make_color(colornum, color);
	tex = (float*)malloc(w*h*4*sizeof(float));
	for(i = 0; i < w*h; ++i) {
		tex[4*i] = color[0];
		tex[4*i + 1] = color[1];
		tex[4*i + 2] = color[2];
		tex[4*i + 3] = color[3];
	}
	glTexImage2D(GL_TEXTURE_2D, lvl, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, tex);
	free(tex);
}

/** Render the given texture level into the given cell and verify the resulting color*/
static int cell(int cellnr, int lvl, GLuint basew, GLuint baseh, int colornr,
	const char* testnamefmt, ...)
{
	float color[4];
	float *readback;
	float tcx = (float)CellSize / basew * (1 << lvl);
	float tcy = (float)CellSize / baseh * (1 << lvl);
	int cellx, celly;
	int x, y;

	make_color(colornr, color);
	cell_coords(cellnr, &cellx, &celly);

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(cellx, celly);
		glTexCoord2f(tcx, 0);
		glVertex2f(cellx+CellSize, celly);
		glTexCoord2f(tcx, tcy);
		glVertex2f(cellx+CellSize, celly+CellSize);
		glTexCoord2f(0, tcy);
		glVertex2f(cellx, celly+CellSize);
	glEnd();

	readback = (float*)malloc(CellSize*CellSize*4*sizeof(float));
	glReadPixels(cellx, celly, CellSize, CellSize, GL_RGBA, GL_FLOAT, readback);
	for(y = 0; y < CellSize; ++y) {
		for(x = 0; x < CellSize; ++x) {
			float *read = readback + 4*(y*CellSize + x);
			float eps = 0.01;
			if (fabs(read[0] - color[0]) > eps ||
			    fabs(read[1] - color[1]) > eps ||
			    fabs(read[2] - color[2]) > eps ||
			    fabs(read[3] - color[3]) > eps) {
				va_list args;
				va_start(args, testnamefmt);
				fprintf(stderr, "Test #%i failed: ", cellnr+1);
				vfprintf(stderr, testnamefmt, args);
				fprintf(stderr, "\n  First failing pixel %i,%i in cell at %d,%d\n", x, y, cellx, celly);
				fprintf(stderr, "  Expected: %f %f %f %f\n", color[0], color[1], color[2], color[3]);
				fprintf(stderr, "  Readback: %f %f %f %f\n", read[0], read[1], read[2], read[3]);
				va_end(args);
				free(readback);
				return 0;
			}
		}
	}
	free(readback);
	return 1;
}

/** Run a couple of tests with the given base texture size */
static int test_size(GLuint basew, GLuint baseh)
{
	int lvls, lvl;
	GLuint w = basew;
	GLuint h = baseh;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	teximage2d(0, w, h, testnr);
	if (!cell(testnr, 0, basew, baseh, testnr,
	          "Nearest filtering, base size %ix%i", basew, baseh))
		return 0;
	testnr++;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	for(lvls = 0; w >= 1 || h >= 1; lvls++, w >>= 1, h >>= 1) {
		if (!w)
			w = 1;
		if (!h)
			h = 1;

		teximage2d(lvls, w, h, testnr+lvls);
	}

	for(lvl = 0; lvl < lvls; ++lvl) {
		if (!cell(testnr+lvl, lvl, basew, baseh, testnr+lvl,
		          "Mip filtering, base size %ix%i, level %i",
		          basew, baseh, lvl))
			return 0;
	}
	testnr += lvls;

	return 1;
}

struct size {
	GLuint w, h;
};

static const struct size sizes[] = {
	{ 16, 16 },
	{ 32, 16 },
	{ 16, 32 },
	{ 32, 32 },
	{ 16, 16 }
};

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int sizeidx;

	testnr = 0;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);

	for(sizeidx = 0; sizeidx < sizeof(sizes)/sizeof(sizes[0]); ++sizeidx) {
		if (!test_size(sizes[sizeidx].w, sizes[sizeidx].h)) {
			pass = GL_FALSE;
		}
	}

	glDisable(GL_TEXTURE_2D);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
}
