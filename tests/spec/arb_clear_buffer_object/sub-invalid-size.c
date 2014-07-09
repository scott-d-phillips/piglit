/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file sub-invalid-size.c
 *
 * From the GL_ARB_clear_buffer_object spec:
 * "If <offset> or <size> is less than zero, or if <offset> + <size> is greater
 *  than the value of BUFFER_SIZE for the buffer bound to <target> then the
 *  error INVALID_VALUE is generated."
 * and
 * "Both <offset> and <range> must be multiples of the number of basic machine
 *  units per-element for that internal format specified by <internalformat>,
 *  otherwise the error INVALID_VALUE is generated."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int i;
	const int buffer_size = 1<<20;
	unsigned int buffer;
	static const char *const data_zero = "\x00\x00\x00\x00\x00\x00\x00\x00";
	ptrdiff_t ranges[][2] = {
		{-8, buffer_size},
		{16, -8},
		{buffer_size - 8, 16},

		/* We're using GL_RGBA16 which is 8 bytes wide. */
		{4, 8},
		{8, 12},
	};

	piglit_require_extension("GL_ARB_clear_buffer_object");

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STREAM_READ);

	for (i = 0; i < ARRAY_SIZE(ranges); ++i) {
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		glClearBufferSubData(GL_ARRAY_BUFFER, GL_RGBA16, ranges[i][0],
				ranges[i][1], GL_RGBA, GL_UNSIGNED_SHORT,
				data_zero);
		pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

