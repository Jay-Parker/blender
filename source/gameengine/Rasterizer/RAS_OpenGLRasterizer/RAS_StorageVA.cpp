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

#include "RAS_StorageVA.h"
#include "RAS_IPolygonMaterial.h"
#include "RAS_DisplayArray.h"
#include "RAS_MaterialBucket.h"

#include "glew-mx.h"

RAS_DisplayList::RAS_DisplayList()
{
	for (unsigned short i = 0; i < RAS_IRasterizer::KX_DRAW_MAX; ++i) {
		for (unsigned short j = 0; j < NUM_LIST; ++j) {
			m_list[i][j] = -1;
		}
	}
}

RAS_DisplayList::~RAS_DisplayList()
{
	for (unsigned short i = 0; i < RAS_IRasterizer::KX_DRAW_MAX; ++i) {
		RemoveAllList((RAS_IRasterizer::DrawType)i);
	}
}

void RAS_DisplayList::RemoveAllList(RAS_IRasterizer::DrawType drawmode)
{
	for (unsigned short j = 0; j < NUM_LIST; ++j) {
		int list = m_list[drawmode][j];
		if (list != -1) {
			glDeleteLists(list, 1);
		}
		m_list[drawmode][j] = -1;
	}
}

void RAS_DisplayList::SetMeshModified(RAS_IRasterizer::DrawType drawmode, bool modified)
{
	if (modified) {
		RemoveAllList(drawmode);
	}
}

bool RAS_DisplayList::Draw(RAS_IRasterizer::DrawType drawmode, LIST_TYPE type)
{
	int list = m_list[drawmode][type];
	if (list == -1) {
		m_list[drawmode][type] = list = glGenLists(1);
		glNewList(list, GL_COMPILE);

		return false;
	}

	glCallList(list);

	return true;
}

void RAS_DisplayList::End(RAS_IRasterizer::DrawType drawmode, LIST_TYPE type)
{
	glEndList();
	glCallList(m_list[drawmode][type]);
}

RAS_StorageVA::RAS_StorageVA()
	:m_drawingmode(RAS_IRasterizer::KX_TEXTURED)
{
}

RAS_StorageVA::~RAS_StorageVA()
{
}

bool RAS_StorageVA::Init()
{
	return true;
}

void RAS_StorageVA::Exit()
{
}

void RAS_StorageVA::BindPrimitives(RAS_DisplayArrayBucket *arrayBucket)
{
	RAS_DisplayList *displayList = GetDisplayList(arrayBucket);
	if (displayList && displayList->Draw(m_drawingmode, RAS_DisplayList::BIND_LIST)) {
		return;
	}

	RAS_DisplayArray *array = arrayBucket->GetDisplayArray();
	static const GLsizei stride = sizeof(RAS_TexVert);
	bool wireframe = m_drawingmode <= RAS_IRasterizer::KX_WIREFRAME;
	RAS_TexVert *vertexarray = array->m_vertex.data();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, stride, vertexarray->getXYZ());
	glNormalPointer(GL_FLOAT, stride, vertexarray->getNormal());

	/*if (!wireframe) {
		std::cout << __func__ << std::endl;
		RAS_IPolyMaterial *polymat = arrayBucket->GetBucket()->GetPolyMaterial();
		RAS_IRasterizer::TexCoGenList *texCoGenList = polymat->GetTexCoGenList();
		RAS_IRasterizer::AttribList *attribList = polymat->GetAttribList();
		EnableTextures(texCoGenList, attribList, true);
		TexCoordPtr(texCoGenList, attribList, vertexarray);
	}*/

	if (displayList) {
		displayList->End(m_drawingmode, RAS_DisplayList::BIND_LIST);
	}
}

void RAS_StorageVA::UnbindPrimitives(RAS_DisplayArrayBucket *arrayBucket)
{
	RAS_DisplayList *displayList = GetDisplayList(arrayBucket);
	if (displayList && displayList->Draw(m_drawingmode, RAS_DisplayList::UNBIND_LIST)) {
		return;
	}

	bool wireframe = m_drawingmode <= RAS_IRasterizer::KX_WIREFRAME;
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	/*if (!wireframe) {
		glDisableClientState(GL_COLOR_ARRAY);
		RAS_IPolyMaterial *polymat = arrayBucket->GetBucket()->GetPolyMaterial();
		EnableTextures(polymat->GetTexCoGenList(), polymat->GetAttribList(), false);
	}*/

	if (displayList) {
		displayList->End(m_drawingmode, RAS_DisplayList::UNBIND_LIST);
	}

}

void RAS_StorageVA::IndexPrimitives(RAS_MeshSlot *ms)
{
	RAS_DisplayArrayBucket *arrayBucket = ms->m_displayArrayBucket;

	RAS_DisplayList *displayList = GetDisplayList(arrayBucket);
	if (displayList && displayList->Draw(m_drawingmode, RAS_DisplayList::DRAW_LIST)) {
		return;
	}

	static const GLsizei stride = sizeof(RAS_TexVert);
	bool wireframe = m_drawingmode <= RAS_IRasterizer::KX_WIREFRAME;
	RAS_DisplayArray *array = ms->GetDisplayArray();
	RAS_TexVert *vertexarray = array->m_vertex.data();
	unsigned int *indexarray = array->m_index.data();
	RAS_IPolyMaterial *material = ms->m_bucket->GetPolyMaterial();

	// colors
	if (!wireframe) {
		if (material->UsesObjectColor()) {
			const MT_Vector4& rgba = ms->m_RGBAcolor;

			glDisableClientState(GL_COLOR_ARRAY);
			glColor4d(rgba[0], rgba[1], rgba[2], rgba[3]);
		}
		else {
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, vertexarray->getRGBA());
		}
	}
	else
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	// here the actual drawing takes places
	glDrawElements(GL_TRIANGLES, array->m_index.size(), GL_UNSIGNED_INT, indexarray);

	if (displayList) {
		displayList->End(m_drawingmode, RAS_DisplayList::DRAW_LIST);
	}
}

RAS_DisplayList *RAS_StorageVA::GetDisplayList(RAS_DisplayArrayBucket *arrayBucket)
{
	if (!arrayBucket->UseDisplayList()) {
		return NULL;
	}

	RAS_DisplayList *displayList = (RAS_DisplayList *)arrayBucket->GetStorageInfo();
	if (!displayList) {
		displayList = new RAS_DisplayList();
		arrayBucket->SetStorageInfo(displayList);
	}

	return displayList;
}

void RAS_StorageVA::TexCoordPtr(RAS_IRasterizer::TexCoGenList *texCoGenList,
								RAS_IRasterizer::AttribList *attribList,
								const RAS_TexVert *tv)
{
	/* note: this function must closely match EnableTextures to enable/disable
	 * the right arrays, otherwise coordinate and attribute pointers from other
	 * materials can still be used and cause crashes */

	if (GLEW_ARB_multitexture && texCoGenList) {
		for (unsigned short unit = 0, size = texCoGenList->size(); unit < size; ++unit) {
			glClientActiveTextureARB(GL_TEXTURE0_ARB + unit);
			switch (texCoGenList->at(unit)) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				{
					glTexCoordPointer(3, GL_FLOAT, sizeof(RAS_TexVert), tv->getXYZ());
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_UV:
				{
					glTexCoordPointer(2, GL_FLOAT, sizeof(RAS_TexVert), tv->getUV(unit));
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				{
					glTexCoordPointer(3, GL_FLOAT, sizeof(RAS_TexVert), tv->getNormal());
					break;
				}
				case RAS_IRasterizer::RAS_TEXTANGENT:
				{
					glTexCoordPointer(4, GL_FLOAT, sizeof(RAS_TexVert), tv->getTangent());
					break;
				}
				default:
					break;
			}
		}
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	if (GLEW_ARB_vertex_program && attribList) {
		for (RAS_IRasterizer::AttribList::iterator it = attribList->begin(), end = attribList->end(); it != end; ++it) {
			const RAS_IRasterizer::Attrib& attrib = (*it);
			const unsigned int unit = attrib.index;
			switch (attrib.texco) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				{
					glVertexAttribPointerARB(unit, 3, GL_FLOAT, GL_FALSE, sizeof(RAS_TexVert), tv->getXYZ());
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_UV:
				{
					glVertexAttribPointerARB(unit, 2, GL_FLOAT, GL_FALSE, sizeof(RAS_TexVert), tv->getUV(attrib.layer));
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				{
					glVertexAttribPointerARB(unit, 3, GL_FLOAT, GL_FALSE, sizeof(RAS_TexVert), tv->getNormal());
					break;
				}
				case RAS_IRasterizer::RAS_TEXTANGENT:
				{
					glVertexAttribPointerARB(unit, 4, GL_FLOAT, GL_FALSE, sizeof(RAS_TexVert), tv->getTangent());
					break;
				}
				case RAS_IRasterizer::RAS_TEXCO_VCOL:
				{
					glVertexAttribPointerARB(unit, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(RAS_TexVert), tv->getRGBA());
					break;
				}
				default:
					break;
			}
		}
	}
}

void RAS_StorageVA::EnableTextures(RAS_IRasterizer::TexCoGenList *texCoGenList,
								   RAS_IRasterizer::AttribList *attribList,
								   bool enable)
{
	if (GLEW_ARB_multitexture && texCoGenList) {
		for (unsigned short unit = 0, size = texCoGenList->size(); unit < size; ++unit) {
			glClientActiveTextureARB(GL_TEXTURE0_ARB + unit);

			switch (texCoGenList->at(unit)) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				case RAS_IRasterizer::RAS_TEXCO_UV:
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				case RAS_IRasterizer::RAS_TEXTANGENT:
				{
					if (enable) {
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					}
					else {
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					}
					break;
				}
				default:
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					break;
				}
			}
		}

		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}
	else {
		if (texCoGenList && texCoGenList->size() > 0) {
			if (enable) {
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			else {
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
	}

	if (GLEW_ARB_vertex_program && attribList) {
		for (RAS_IRasterizer::AttribList::iterator it = attribList->begin(), end = attribList->end(); it != end; ++it) {
			const RAS_IRasterizer::Attrib& attrib = (*it);
			const unsigned int unit = attrib.index;
			switch (attrib.texco) {
				case RAS_IRasterizer::RAS_TEXCO_ORCO:
				case RAS_IRasterizer::RAS_TEXCO_GLOB:
				case RAS_IRasterizer::RAS_TEXCO_UV:
				case RAS_IRasterizer::RAS_TEXCO_NORM:
				case RAS_IRasterizer::RAS_TEXTANGENT:
				case RAS_IRasterizer::RAS_TEXCO_VCOL:
				{
					if (enable) {
						glEnableVertexAttribArrayARB(unit);
					}
					else {
						glDisableVertexAttribArrayARB(unit);
					}
					break;
				}
				default:
				{
					glDisableVertexAttribArrayARB(unit);
					break;
				}
			}
		}
	}
}
