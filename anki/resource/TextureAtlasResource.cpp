// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/TextureAtlasResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Xml.h>

namespace anki
{

TextureAtlasResource::TextureAtlasResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

TextureAtlasResource::~TextureAtlasResource()
{
	m_subTexes.destroy(getAllocator());
	m_subTexNames.destroy(getAllocator());
}

Error TextureAtlasResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement rootel, el;

	//
	// <textureAtlas>
	//
	ANKI_CHECK(doc.getChildElement("textureAtlas", rootel));

	//
	// <texture>
	//
	ANKI_CHECK(rootel.getChildElement("texture", el));
	CString texFname;
	ANKI_CHECK(el.getText(texFname));
	ANKI_CHECK(getManager().loadResource<TextureResource>(texFname, m_tex, async));

	m_size[0] = m_tex->getWidth();
	m_size[1] = m_tex->getHeight();

	//
	// <subTextureMargin>
	//
	ANKI_CHECK(rootel.getChildElement("subTextureMargin", el));
	I64 margin = 0;
	ANKI_CHECK(el.getNumber(margin));
	if(margin >= I(m_tex->getWidth()) || margin >= I(m_tex->getHeight()) || margin < 0)
	{
		ANKI_RESOURCE_LOGE("Too big margin %d", I32(margin));
		return Error::USER_DATA;
	}
	m_margin = U32(margin);

	//
	// <subTextures>
	//

	// Get counts
	U32 namesSize = 0;
	U32 subTexesCount = 0;
	XmlElement subTexesEl, subTexEl;
	ANKI_CHECK(rootel.getChildElement("subTextures", subTexesEl));
	ANKI_CHECK(subTexesEl.getChildElement("subTexture", subTexEl));
	do
	{
		ANKI_CHECK(subTexEl.getChildElement("name", el));
		CString name;
		ANKI_CHECK(el.getText(name));

		if(name.getLength() < 1)
		{
			ANKI_RESOURCE_LOGE("Something wrong with the <name> tag. Probably empty");
			return Error::USER_DATA;
		}

		namesSize += U32(name.getLength()) + 1;
		++subTexesCount;

		ANKI_CHECK(subTexEl.getNextSiblingElement("subTexture", subTexEl));
	} while(subTexEl);

	// Allocate
	m_subTexNames.create(getAllocator(), namesSize);
	m_subTexes.create(getAllocator(), subTexesCount);

	// Iterate again and populate
	subTexesCount = 0;
	char* names = &m_subTexNames[0];
	ANKI_CHECK(subTexesEl.getChildElement("subTexture", subTexEl));
	do
	{
		ANKI_CHECK(subTexEl.getChildElement("name", el));
		CString name;
		ANKI_CHECK(el.getText(name));

		memcpy(names, &name[0], name.getLength() + 1);

		m_subTexes[subTexesCount].m_name = names;

		ANKI_CHECK(subTexEl.getChildElement("uv", el));
		Vec4 uv;
		ANKI_CHECK(el.getNumbers(uv));
		m_subTexes[subTexesCount].m_uv = {uv[0], uv[1], uv[2], uv[3]};

		names += name.getLength() + 1;
		++subTexesCount;

		ANKI_CHECK(subTexEl.getNextSiblingElement("subTexture", subTexEl));
	} while(subTexEl);

	return Error::NONE;
}

Error TextureAtlasResource::getSubTextureInfo(CString name, F32 uv[4]) const
{
	for(const SubTex& st : m_subTexes)
	{
		if(st.m_name == name)
		{
			uv[0] = st.m_uv[0];
			uv[1] = st.m_uv[1];
			uv[2] = st.m_uv[2];
			uv[3] = st.m_uv[3];
			return Error::NONE;
		}
	}

	ANKI_RESOURCE_LOGE("Texture atlas %s doesn't have sub texture named: %s", &getFilename()[0], &name[0]);
	return Error::USER_DATA;
}

} // end namespace anki
