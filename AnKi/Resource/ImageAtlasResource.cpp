// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ImageAtlasResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Xml.h>

namespace anki {

ImageAtlasResource::ImageAtlasResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ImageAtlasResource::~ImageAtlasResource()
{
	m_subTexes.destroy(getMemoryPool());
	m_subTexNames.destroy(getMemoryPool());
}

Error ImageAtlasResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc(&getTempMemoryPool());
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement rootel, el;

	//
	// <imageAtlas>
	//
	ANKI_CHECK(doc.getChildElement("imageAtlas", rootel));

	//
	// <image>
	//
	ANKI_CHECK(rootel.getChildElement("image", el));
	CString texFname;
	ANKI_CHECK(el.getText(texFname));
	ANKI_CHECK(getManager().loadResource<ImageResource>(texFname, m_image, async));

	m_size[0] = m_image->getWidth();
	m_size[1] = m_image->getHeight();

	//
	// <subImageMargin>
	//
	ANKI_CHECK(rootel.getChildElement("subImageMargin", el));
	I64 margin = 0;
	ANKI_CHECK(el.getNumber(margin));
	if(margin >= I(m_image->getWidth()) || margin >= I(m_image->getHeight()) || margin < 0)
	{
		ANKI_RESOURCE_LOGE("Too big margin %d", I32(margin));
		return Error::kUserData;
	}
	m_margin = U32(margin);

	//
	// <subImages>
	//

	// Get counts
	U32 namesSize = 0;
	U32 subTexesCount = 0;
	XmlElement subTexesEl, subTexEl;
	ANKI_CHECK(rootel.getChildElement("subImages", subTexesEl));
	ANKI_CHECK(subTexesEl.getChildElement("subImage", subTexEl));
	do
	{
		ANKI_CHECK(subTexEl.getChildElement("name", el));
		CString name;
		ANKI_CHECK(el.getText(name));

		if(name.getLength() < 1)
		{
			ANKI_RESOURCE_LOGE("Something wrong with the <name> tag. Probably empty");
			return Error::kUserData;
		}

		namesSize += U32(name.getLength()) + 1;
		++subTexesCount;

		ANKI_CHECK(subTexEl.getNextSiblingElement("subImage", subTexEl));
	} while(subTexEl);

	// Allocate
	m_subTexNames.create(getMemoryPool(), namesSize);
	m_subTexes.create(getMemoryPool(), subTexesCount);

	// Iterate again and populate
	subTexesCount = 0;
	char* names = &m_subTexNames[0];
	ANKI_CHECK(subTexesEl.getChildElement("subImage", subTexEl));
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

		ANKI_CHECK(subTexEl.getNextSiblingElement("subImage", subTexEl));
	} while(subTexEl);

	return Error::kNone;
}

Error ImageAtlasResource::getSubImageInfo(CString name, F32 uv[4]) const
{
	for(const SubTex& st : m_subTexes)
	{
		if(st.m_name == name)
		{
			uv[0] = st.m_uv[0];
			uv[1] = st.m_uv[1];
			uv[2] = st.m_uv[2];
			uv[3] = st.m_uv[3];
			return Error::kNone;
		}
	}

	ANKI_RESOURCE_LOGE("Image atlas %s doesn't have sub image named: %s", &getFilename()[0], &name[0]);
	return Error::kUserData;
}

} // end namespace anki
