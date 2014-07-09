/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file negative-max-samples.c
 *
 * Tests that asking for more that GL_MAX_SAMPLES fails.
 *
 * From the EXT_framebuffer_multisample spec:
 *
 *     "If either <width> or <height> is greater than
 *      MAX_RENDERBUFFER_SIZE_EXT, or if <samples> is greater than
 *      MAX_SAMPLES_EXT, then the error INVALID_VALUE is generated."
 *
 * Skips if ARB_texture_multisample or ARB_internalformat_query are
 * supported. ARB_texture_multisample changes the error which should
 * be generated; ARB_internalformat_query allows the limit for particular
 * internalformats to be >MAX_SAMPLES.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_samples;
	GLuint rb;

	piglit_require_extension("GL_EXT_framebuffer_multisample");

	if (piglit_is_extension_supported("GL_ARB_internalformat_query")) {
		printf("ARB_internalformat_query is supported and "
		       "redefines this behavior; skipping\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	if (piglit_is_extension_supported("GL_ARB_texture_multisample")) {
		printf("ARB_texture_multisample is supposed and "
		       "redefines this behavior; skipping\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER, rb);

	glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
					    max_samples + 1,
					    GL_RGBA,
					    1, 1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteRenderbuffersEXT(1, &rb);

	piglit_report_result(PIGLIT_PASS);
}
