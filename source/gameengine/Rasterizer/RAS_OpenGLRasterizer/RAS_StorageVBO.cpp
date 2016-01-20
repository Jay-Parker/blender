/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include "RAS_StorageVBO.h"
#include "RAS_DisplayArray.h"
#include "RAS_MeshObject.h"
#include "RAS_Deformer.h"

#include "glew-mx.h"

VBO::VBO(RAS_DisplayArray *data, unsigned int indices)
{
	m_data = data;
	m_size = data->m_vertex.size();
	m_indices = indices;
	m_stride = sizeof(RAS_TexVert);

	m_mode = GL_TRIANGLES;

	// Generate Buffers
	glGenBuffersARB(1, &m_ibo);
	glGenBuffersARB(1, &m_vbo_id);

	// Fill the buffers with initial data
	UpdateIndices();
	UpdateData();

	// Establish offsets
	m_vertex_offset = (void *)(((RAS_TexVert *)0)->getXYZ());
	m_normal_offset = (void *)(((RAS_TexVert *)0)->getNormal());
	m_tangent_offset = (void *)(((RAS_TexVert *)0)->getTangent());
	m_color_offset = (void *)(((RAS_TexVert *)0)->getRGBA());
	m_uv_offset = (void *)(((RAS_TexVert *)0)->getUV(0));
}

VBO::~VBO()
{
	glDeleteBuffersARB(1, &m_ibo);
	glDeleteBuffersARB(1, &m_vbo_id);
}

void VBO::UpdateData()
{
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo_id);
	glBufferData(GL_ARRAY_BUFFER, m_stride * m_size, m_data->m_vertex.data(), GL_STATIC_DRAW);
}

void VBO::UpdateIndices()
{
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_data->m_index.size() * sizeof(GLuint),
	             m_data->m_index.data(), GL_STATIC_DRAW);
}

void VBO::Bind(RAS_IRasterizer::TexCoGenList *texCoGenList, RAS_IRasterizer::AttribList *attribList)
{
	// Bind buffers
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo_id);

	// Vertexes
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, m_stride, m_vertex_offset);

	// Normals
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, m_stride, m_normal_offset);

	// Colors
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, m_stride, m_color_offset);

	if (texCoGenList) {
		for (unsigned short unit = 0, size = texCoGenList->size(); unit < size; ++unit) {
			glClientActiveTexture(GL_TEXTURE0_ARB + unit);
			switch (texCoGenList->at(unit)) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(3, GL_FLOAT, m_stride, m_vertex_offset);
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_UV:
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(2, GL_FLOAT, m_stride, (void *)((intptr_t)m_uv_offset + (sizeof(GLfloat) * 2 * unit)));
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(3, GL_FLOAT, m_stride, m_normal_offset);
					break;
				}
				case RAS_IRasterizer::RAS_TEXTANGENT:
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glTexCoordPointer(4, GL_FLOAT, m_stride, m_tangent_offset);
					break;
				}
				default:
					break;
			}
		}
	}
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	if (GLEW_ARB_vertex_program && attribList) {
		for (unsigned short unit = 0, size = attribList->size(); unit < size; ++unit) {
			switch (attribList->at(unit).texco) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				{
					glVertexAttribPointerARB(unit, 3, GL_FLOAT, GL_FALSE, m_stride, m_vertex_offset);
					glEnableVertexAttribArrayARB(unit);
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_UV:
				{
					glVertexAttribPointerARB(unit, 2, GL_FLOAT, GL_FALSE, m_stride, 
							(void *)((intptr_t)m_uv_offset + attribList->at(unit).layer * sizeof(GLfloat) * 2));
					glEnableVertexAttribArrayARB(unit);
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				{
					glVertexAttribPointerARB(unit, 2, GL_FLOAT, GL_FALSE, m_stride, m_normal_offset);
					glEnableVertexAttribArrayARB(unit);
					break;
				}
				case RAS_IRasterizer::RAS_TEXTANGENT:
				{
					glVertexAttribPointerARB(unit, 4, GL_FLOAT, GL_FALSE, m_stride, m_tangent_offset);
					glEnableVertexAttribArrayARB(unit);
					break;
				}
				default:
					break;
			}
		}
	}
}

void VBO::Unbind(RAS_IRasterizer::TexCoGenList *texCoList, RAS_IRasterizer::AttribList *attribList)
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (GLEW_ARB_vertex_program && attribList) {
		for (unsigned short unit = 0, size = attribList->size(); unit < size; ++unit) {
			glDisableVertexAttribArrayARB(unit);
		}
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void VBO::Draw()
{
	glDrawElements(m_mode, m_indices, GL_UNSIGNED_INT, 0);
}

RAS_StorageVBO::RAS_StorageVBO()
	:m_drawingmode(RAS_IRasterizer::KX_TEXTURED)
{
}

RAS_StorageVBO::~RAS_StorageVBO()
{
}

bool RAS_StorageVBO::Init()
{
	return true;
}

void RAS_StorageVBO::Exit()
{
}

VBO *RAS_StorageVBO::GetVBO(RAS_DisplayArrayBucket *arrayBucket)
{
	VBO *vbo = (VBO *)arrayBucket->GetStorageInfo();
	if (!vbo) {
		RAS_DisplayArray *array = arrayBucket->GetDisplayArray();
		vbo = new VBO(array, array->m_index.size());
		arrayBucket->SetStorageInfo(vbo);
	}
	return vbo;
}

void RAS_StorageVBO::BindPrimitives(RAS_DisplayArrayBucket *arrayBucket)
{
	VBO *vbo = GetVBO(arrayBucket);
	RAS_IPolyMaterial *polymat = arrayBucket->GetBucket()->GetPolyMaterial();
	vbo->Bind(polymat->GetTexCoGenList(), polymat->GetAttribList());
}

void RAS_StorageVBO::UnbindPrimitives(RAS_DisplayArrayBucket *arrayBucket)
{
	VBO *vbo = GetVBO(arrayBucket);
	RAS_IPolyMaterial *polymat = arrayBucket->GetBucket()->GetPolyMaterial();
	vbo->Unbind(polymat->GetTexCoGenList(), polymat->GetAttribList());
}

void RAS_StorageVBO::IndexPrimitives(RAS_MeshSlot *ms)
{
	RAS_DisplayArrayBucket *arrayBucket = ms->m_displayArrayBucket;
	VBO *vbo = GetVBO(arrayBucket);

	// Update the vbo if the mesh is modified or use a dynamic deformer.
	if (arrayBucket->IsMeshModified()) {
		vbo->UpdateData();
	}

	vbo->Draw();
}
