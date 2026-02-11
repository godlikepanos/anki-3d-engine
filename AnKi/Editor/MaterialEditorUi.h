// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Editor/EditorCommon.h>
#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

class MaterialEditorUi : public EditorUiBase
{
public:
	void open(const MaterialResource& resource);

	void drawWindow(Vec2 initialPos, Vec2 initialSize, ImGuiWindowFlags windowFlags);

private:
	class Data
	{
	public:
		union
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.def.h>
		};

		ImageResourcePtr m_image;
		String m_name;
		ShaderVariableDataType m_type = ShaderVariableDataType::kCount;

		Data()
		{
			m_Mat4 = Mat4::getZero();
		}
	};

	class Program
	{
	public:
		ShaderBinary* m_programBinary = nullptr; // Keep it alive because following members point to the binary
		String m_name;

		ConstWeakArray<ShaderBinaryMutator> m_mutators;

		ConstWeakArray<ShaderBinaryStructMember> m_inputs;

		Program() = default;

		Program(const Program&) = delete;

		Program(Program&& b)
		{
			*this = std::move(b);
		}

		~Program()
		{
			DefaultMemoryPool::getSingleton().free(m_programBinary);
		}

		Program& operator=(const Program&) = delete;

		Program& operator=(Program&& b)
		{
			m_programBinary = b.m_programBinary;
			b.m_programBinary = nullptr;
			m_name = std::move(b.m_name);
			m_mutators = std::move(b.m_mutators);
			m_inputs = std::move(b.m_inputs);
			return *this;
		}
	};

	Bool m_open = false;

	DynamicArray<Program> m_programs;

	ImageResourcePtr m_placeholderImage;

	ImGuiTextFilter m_tempFilter;

	// Cache begins
	String m_filepath;
	DynamicArray<PartialMutation> m_cachedMutators;
	DynamicArray<Data> m_cachedInputs;
	String m_currentlySelectedProgram;
	// Cache ends

	void gatherPrograms();

	void rebuildCache(const MaterialResource& mtl);
	void rebuildCache(CString programName);

	Error saveCache();
};

} // end namespace anki
