/*
 * Copyright © 2012 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file getuniformindices.c
 *
 * Tests the glGetUniformIndices API: not writing on error conditions,
 * invalid uniform names, and consecutive indices starting from 0.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_ALPHA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const char frag_shader_text[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"uniform ub_a { vec4 a; vec4 b; };\n"
	"uniform vec4 c;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = a + b + c;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint fs;
	GLuint save_index = 0xaaaaaaaa;
	const GLchar *one_uniform = "a";
	const GLchar *bad_uniform = "d";
	const GLchar *uniform_names[] = {"a", "b", "c"};
	bool found_index[3] = {false, false, false};
	GLuint indices[3], index;
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader_text);
	if (!fs) {
		printf("Failed to compile FS:\n%s", frag_shader_text);
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = piglit_link_simple_program(0, fs);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	/* From the GL_ARB_uniform_buffer_object spec:
	 *
	 *     "The error INVALID_VALUE is generated by GetUniformIndices,
	 *      GetActiveUniformsiv, GetActiveUniformName, GetUniformBlockIndex,
	 *      GetActiveUniformBlockiv, GetActiveUniformBlockName, and
	 *      UniformBlockBinding if <program> is not a value generated by GL.
	 *
	 *      ...
	 *
	 *      The error INVALID_VALUE is generated by GetUniformIndices and
	 *      GetActiveUniformsiv if <uniformCount> is less than zero.
	 *
	 *      ...
	 *
	 *     "If an error occurs, nothing is written to <uniformIndices>."
	 */
	index = save_index;
	glGetUniformIndices(prog, -1, &one_uniform, &index);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		pass = false;
	} else if (index != save_index) {
		printf("Bad program uniform index: 0x%08x\n", index);
		printf("  Expected 0x%08x\n", save_index);
		pass = false;
	}

	index = save_index;
	glGetUniformIndices(0xd0d0, 1, &one_uniform, &index);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		pass = false;
	} else if (index != save_index) {
		printf("Bad program uniform index: 0x%08x\n", index);
		printf("  Expected 0x%08x\n", save_index);
		pass = false;
	}

	glGetUniformIndices(prog, 3, uniform_names, indices);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	for (i = 0; i < 3; i++) {
		printf("%s: index %d\n", uniform_names[i], indices[i]);
		if (indices[i] > 2 || found_index[indices[i]]) {
			printf("Expected consecutive numbers starting from 0\n");
			pass = false;
		}
		found_index[indices[i]] = true;
	}

	/*     "If a string in <uniformNames> is not the name of an
	 *      active uniform, the value INVALID_INDEX will be
	 *      written to the corresponding element of
	 *      <uniformIndices>."
	 */
	glGetUniformIndices(prog, 1, &bad_uniform, &index);
	if (!piglit_check_gl_error(0)) {
		pass = false;
	} else if (index != GL_INVALID_INDEX) {
		printf("Bad uniform index for %s: 0x%08x\n", bad_uniform, index);
		printf("  Expected 0x%08x\n", GL_INVALID_INDEX);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
