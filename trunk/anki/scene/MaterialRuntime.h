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
/// This holds a copy of the MaterialVariable's data in order to be read write
/// inside the main loop
class MaterialRuntimeVariable
{
public:
	/// The data union. The Texture resource is read-only at runtime
	/// Don't EVER replace the texture with const Texture*. The asynchronous
	/// operations will fail
	typedef MaterialVariable::Variant Variant;

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

	const Variant& getVariant() const
	{
		return getVariantConst();
	}
	Variant& getVariant()
	{
		return getVariantMutable();
	}

	/// Get the value of the variant
	/// @exception boost::exception when you try to get the incorrect data
	/// type
	template<typename Type>
	const Type& getValue() const
	{
		return boost::get<Type>(getVariantConst());
	}

	/// @copydoc getValue
	template<typename Type>
	Type& getValue()
	{
		return boost::get<Type>(getVariantMutable());
	}

	template<typename Type>
	void setValue(const Type& v)
	{
		boost::get<Type>(getVariantMutable()) = v;
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

private:
	const MaterialVariable& mvar; ///< Know the resource
	boost::scoped_ptr<Variant> copyVariant; /// The copy of the data
	int buildinId; ///< -1 is "dont know yet"

	/// Return either the mvar's variant or your own if exists
	const Variant& getVariantConst() const;

	/// Implement the COW technique. If someone asks the variant for writing
	/// then create a copy
	Variant& getVariantMutable();
};


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
