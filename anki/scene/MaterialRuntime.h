#ifndef ANKI_SCENE_MATERIAL_RUNTIME_H
#define ANKI_SCENE_MATERIAL_RUNTIME_H

#include "anki/resource/Material.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/resource/Resource.h"
#include "anki/math/Math.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/variant.hpp>


namespace anki {


class Texture;
class MaterialVariable;


/// Variable of runtime materials
/// This holds a copy of the MtlUserDefinedVar's data in order to be changed
/// inside the main loop
class MaterialRuntimeVariable
{
public:
	typedef const TextureResourcePointer* ConstPtrRsrcPtrTexture;

	/// The data union. The Texture resource is read-only at runtime
	/// Don't EVER replace the texture with const Texture*. The asynchronous
	/// operations will fail
	typedef boost::variant<float, Vec2, Vec3, Vec4, Mat3,
		Mat4, ConstPtrRsrcPtrTexture> Variant;

	/// Constructor. Initialize using a material variable
	MaterialRuntimeVariable(const MaterialVariable& mv);

	/// Destructor
	~MaterialRuntimeVariable();

	/// @name Accessors
	/// @{
	const MaterialVariable& getMaterialVariable() const
	{
		return mvar;
	}

	const Variant& getDataVariant() const
	{
		return data;
	}
	Variant& getDataVariant()
	{
		return data;
	}

	/// Get the value of the variant
	/// @exception boost::exception when you try to get the incorrect data
	/// type
	template<typename Type>
	const Type& getValue() const
	{
		return boost::get<Type>(data);
	}

	/// Get the value of the variant
	/// @exception boost::exception when you try to get the incorrect data
	/// type
	template<typename Type>
	Type& getValue()
	{
		return boost::get<Type>(data);
	}

	template<typename Type>
	void setValue(const Type& v)
	{
		boost::get<Type>(data) = v;
	}

	int getBuildinId() const
	{
		return buildinId;
	}
	int& getBuildinId()
	{
		return buildinId;
	}
	void setBuildinId(const int x)
	{
		buildinId = x;
	}
	/// @}

	/// Call one of the setters of the uniform variable
	void setUniformVariable(const PassLevelKey& k, uint& texUnif);

private:
	/// Initialize the data using a visitor
	class ConstructVisitor: public boost::static_visitor<void>
	{
	public:
		MaterialRuntimeVariable& var;

		ConstructVisitor(MaterialRuntimeVariable& var_)
			: var(var_)
		{}

		/// Template method that applies to all Variant values except texture
		/// resource
		template<typename Type>
		void operator()(const Type& x) const
		{
			var.getDataVariant() = x;
		}
	};

	/// Set visitor
	class SetUniformVisitor: public boost::static_visitor<void>
	{
	public:
		const ShaderProgramUniformVariable& uni;
		uint& texUnit;

		SetUniformVisitor(const ShaderProgramUniformVariable& uni_,
			uint& texUnit_)
			: uni(uni_), texUnit(texUnit_)
		{}

		template<typename Type>
		void operator()(const Type& x) const;
	};

	const MaterialVariable& mvar; ///< Know the resource
	Variant data; /// The data
	int buildinId;
};


// Declare specialized
template <>
void MaterialRuntimeVariable::ConstructVisitor::
	operator()<TextureResourcePointer >(const TextureResourcePointer& x) const;

// Declare specialized
template<>
MaterialRuntimeVariable::ConstPtrRsrcPtrTexture&
	MaterialRuntimeVariable::getValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>();

// Declare specialized
template<>
void MaterialRuntimeVariable::setValue<
	MaterialRuntimeVariable::ConstPtrRsrcPtrTexture>(
	const ConstPtrRsrcPtrTexture& v);


/// One layer above the material resource
class MaterialRuntime: public MaterialProperties
{
	public:
		/// A type
		typedef boost::ptr_vector<MaterialRuntimeVariable>
			VariablesContainer;

		typedef boost::iterator_range<VariablesContainer::const_iterator>
			ConstIteratorRange;
		typedef boost::iterator_range<VariablesContainer::iterator>
			MutableIteratorRange;

		typedef ConstCharPtrHashMap<MaterialRuntimeVariable*>::Type
			VariablesHashMap;

		/// @name Constructors & destructor
		/// @{
		MaterialRuntime(const Material& mtl);
		~MaterialRuntime();
		/// @}

		/// @name Accessors
		/// @{
		bool getShadow() const
		{
			return shadow;
		}
		bool& getShadow()
		{
			return shadow;
		}
		void setShadow(bool x)
		{
			shadow = x;
		}

		int getBlendingSFactor() const
		{
			return blendingSfactor;
		}
		int& getBlendingSFactor()
		{
			return blendingSfactor;
		}
		void setBlendingSFactor(int x)
		{
			blendingSfactor = x;
		}

		int getBlendingDFactor() const
		{
			return blendingDfactor;
		}
		int& getBlendingDFactor()
		{
			return blendingDfactor;
		}
		void setBlendingDFactor(int x)
		{
			blendingDfactor = x;
		}

		bool getDepthTesting() const
		{
			return depthTesting;
		}
		bool& getDepthTesting()
		{
			return depthTesting;
		}
		void setDepthTesting(bool x)
		{
			depthTesting = x;
		}

		bool getWireframe() const
		{
			return wireframe;
		}
		bool& getWireframe()
		{
			return wireframe;
		}
		void setWireframe(bool x)
		{
			wireframe = x;
		}

		ConstIteratorRange getVariables() const
		{
			return ConstIteratorRange(vars.begin(), vars.end());
		}
		MutableIteratorRange getVariables()
		{
			return MutableIteratorRange(vars.begin(), vars.end());
		}

		const Material& getMaterial() const
		{
			return mtl;
		}
		/// @}

		/// Find a material runtime variable. On failure it throws an exception
		/// @param[in] name The name of the var
		/// @return It returns a MaterialRuntimeUserDefinedVar
		/// @exception Exception
		MaterialRuntimeVariable& findVariableByName(const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const MaterialRuntimeVariable& findVariableByName(
			const char* name) const;

		bool variableExists(const char* name) const
		{
			return varNameToVar.find(name) != varNameToVar.end();
		}

	private:
		const Material& mtl; ///< The resource
		VariablesContainer vars;
		VariablesHashMap varNameToVar;
};


} // end namespace


#endif
