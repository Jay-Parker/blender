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
 * Contributor(s): Pierluigi Grassi, Porteries Tristan.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file RAS_2DFilterManager.h
 *  \ingroup bgerast
 */

#ifndef __RAS_2DFILTERMANAGER_H__
#define __RAS_2DFILTERMANAGER_H__

#include "RAS_2DFilterData.h"
#include <map>

class RAS_ICanvas;
class RAS_2DFilter;

typedef std::map<unsigned int, RAS_2DFilter *> RAS_PassTo2DFilter;

class RAS_2DFilterManager
{
public:
	enum FILTER_MODE {
		FILTER_ENABLED = -2,
		FILTER_DISABLED = -1,
		FILTER_NOFILTER = 0,
		FILTER_MOTIONBLUR,
		FILTER_BLUR,
		FILTER_SHARPEN,
		FILTER_DILATION,
		FILTER_EROSION,
		FILTER_LAPLACIAN,
		FILTER_SOBEL,
		FILTER_PREWITT,
		FILTER_GRAYSCALE,
		FILTER_SEPIA,
		FILTER_INVERT,
		FILTER_CUSTOMFILTER,
		FILTER_NUMBER_OF_FILTERS
	};

	RAS_2DFilterManager(RAS_ICanvas *canvas);
	virtual ~RAS_2DFilterManager();

	void PrintShaderError(unsigned int shaderUid, const char *title, const char *shaderCode, unsigned int passindex);

	/// Applies the filters to the scene.
	void RenderFilters();

	/// Add a filter to the stack of filters managed by this object.
	RAS_2DFilter *AddFilter(RAS_2DFilterData& filterData);

	/// Enables all the filters with pass index info.passIndex.
	void EnableFilterPass(unsigned int passIndex);

	/// Disables all the filters with pass index info.passIndex.
	void DisableFilterPass(unsigned int passIndex);

	/// Removes the filters at a given pass index.
	void RemoveFilterPass(unsigned int passIndex);

	/// Get the existing filter for the given pass index.
	RAS_2DFilter *GetFilterPass(unsigned int passIndex);

	RAS_ICanvas *GetCanvas();

private:
	RAS_PassTo2DFilter m_filters;
	RAS_ICanvas *m_canvas;

	/** Creates a filter matching the given filter data. Returns NULL if no
	 * filter can be created with such information.
	 */
	RAS_2DFilter *CreateFilter(RAS_2DFilterData& filterData);
};

#endif // __RAS_2DFILTERMANAGER_H__
