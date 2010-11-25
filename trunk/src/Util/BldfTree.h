#ifndef BLDF_TREE_H
#define BLDF_TREE_H

#include "Properties.h"
#include "Vec.h"


namespace Bldf {


/// Entity as a base class
class Entity
{
	public:
		enum EntityType
		{
			ET_KEY,
			ET_VALUE
		};

	PROPERTY_R(int, line, getLine)
	PROPERTY_R(EntityType, type, getType)

	public:
		Entity(int line, EntityType type);
};


inline Entity::Entity(int line_, EntityType type_):
	line(line_),
	type(type_)
{}


/// Value
class Value: public Entity
{
	public:
		/// The type of the value
		enum ValueType
		{
			VT_STRING,
			VT_NUMBER,
			VT_BOOL
		};

	PROPERTY_R(ValueType, type, getType) ///< Type

	public:
		Value(int line, const char* str);
		Value(int line, double num);
		Value(int line, bool b);

	private:
		std::string str;
		double num;
		bool b;
};


inline Value::Value(int line, const char* str_):
	Entity(line, ET_VALUE),
	type(VT_STRING),
	str(str_)
{}


inline Value::Value(int line, double num_):
	Entity(line, ET_VALUE),
	type(VT_NUMBER),
	num(num_)
{}


inline Value::Value(int line, bool b_):
	Entity(line, ET_VALUE),
	type(VT_BOOL),
	b(b_)
{}


///
class Key: public Element
{
	PROPERTY_R(std::string, name, getName)
	PROPERTY_R(Vec<Key>, keys, getKeys)
	PROPERTY_R(Vec<Value>, values, getValues)

	public:
		Key(int line): Element(line, ET_KEY) {}
};


/// Bracket loving data file (aka BLDF) tree structure with parser
class Tree
{

};


} // end namespace Bldf

#endif
