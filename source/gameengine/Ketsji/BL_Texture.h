
/** \file BL_Texture.h
 *  \ingroup ketsji
 */

#ifndef __BL_TEXTURE_H__
#define __BL_TEXTURE_H__

struct Image;
struct EnvMap;
class BL_Material;

class BL_Texture
{
private:
	unsigned int m_texture; // Bound texture unit data
	bool m_ok;
	unsigned int m_type; // enum TEXTURE_2D | CUBE_MAP
	unsigned int m_envState; // cache textureEnv
	static unsigned int m_disableState; // speed up disabling calls

public:
	BL_Texture();
	~BL_Texture();

	bool Ok();

	unsigned int GetTextureType() const;
	void DeleteTex();

	void Init(Image *img, bool cubemap);

	bool IsValid();
	void Validate();

	static void ActivateFirst();
	static void DisableAllTextures();
	static void ActivateUnit(int unit);
	static int GetMaxUnits();
	static void SplitEnvMap(EnvMap *map);

	void ActivateTexture(int unit);
	void SetMapping(int mode);
	void DisableUnit(int unit);
	void setTexEnv(int unit, BL_Material *mat, bool modulate = false);
	unsigned int swapTexture(unsigned int newTex)
	{
		// swap texture codes
		unsigned int tmp = m_texture;
		m_texture = newTex;
		// return original texture code
		return tmp;
	}

#ifdef WITH_CXX_GUARDEDALLOC
	MEM_CXX_CLASS_ALLOC_FUNCS("GE:BL_Texture")
#endif
};

#endif // __BL_TEXTURE_H__
