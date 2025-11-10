// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui.h>
#include <AnKi/Resource/ParticleEmitterResource2.h>

namespace anki {

class ParticleEditorUi
{
public:
	void open(const ParticleEmitterResource2& resource);

	void drawWindow(UiCanvas& canvas, Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags = 0);

private:
	class Prop
	{
	public:
		union
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.def.h>
		};

		String m_name;
		ShaderVariableDataType m_type = ShaderVariableDataType::kNone;

		Prop()
		{
			m_Mat4 = Mat4(0.0f);
		}

		Prop(const Prop& b)
		{
			*this = b;
		}

		Prop& operator=(const Prop& b)
		{
			m_name = b.m_name;
			m_type = b.m_type;
			m_Mat4 = b.m_Mat4;
			return *this;
		}
	};

	class ParticleProgram
	{
	public:
		String m_filename;
		String m_name;
		DynamicArray<Prop> m_props;
	};

	Bool m_open = false;

	DynamicArray<ParticleProgram> m_programs;

	String m_filename;

	// Cache begin. The UI will manipulate this cache because the resource is immutable
	String m_currentlySelectedProgram;
	ParticleEmitterResourceCommonProperties m_commonProps = {};
	DynamicArray<Prop> m_otherProps;
	// Cache end

	// Look at the filesystem and get the programs that are for particles
	void gatherParticlePrograms();

	void rebuildCache(const ParticleEmitterResource2& resource);

	void rebuildCache(CString particleProgramName);

	Error saveCache();
};

} // end namespace anki
