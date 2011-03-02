#ifndef PROPERTIES_H
#define PROPERTIES_H


/// Read only getter, cause we are to bored to write
#define GETTER_R(type__, var__, getter__) \
	const type__& getter__() const {return var__;}


/// Read only getter, cause we are to bored to write
#define GETTER_R_BY_VAL(type__, var__, getter__) \
	type__ getter__() const {return var__;}


/// Read-write getter, cause we are to bored to write
#define GETTER_RW(type__, var__, getter__) \
	type__& getter__() {return var__;} \
	GETTER_R(type__, var__, getter__)


/// Read-write getter, cause we are to bored to write
#define GETTER_RW_BY_VAL(type__, var__, getter__) \
	type__& getter__() {return var__;} \
	GETTER_R_BY_VAL(type__, var__, getter__)


/// Setter, cause we are to bored to write
#define SETTER(type__, var__, setter__) \
	void setter__(const type__& x__) {var__ = x__;}


/// Setter, cause we are to bored to write
#define SETTER_BY_VAL(type__, var__, setter__) \
	void setter__(type__ x__) {var__ = x__;}


/// The macro implies read write var
#define GETTER_SETTER(type__, var__, getter__, setter__) \
	GETTER_RW(type__, var__, getter__) \
	SETTER(type__, var__, setter__)


/// The macro implies read write var
#define GETTER_SETTER_BY_VAL(type__, var__, getter__, setter__) \
	GETTER_RW_BY_VAL(type__, var__, getter__) \
	SETTER_BY_VAL(type__, var__, setter__)


/// Read write property
///
/// - It deliberately does not work with pointers
/// - The get funcs are coming into two flavors, one const and one non-const. The property is read-write after all so
///   the non-const is acceptable
/// - Dont use it with semicolon at the end (eg PROPERTY_RW(....);) because of a doxygen bug
#define PROPERTY_RW(Type__, varName__, getFunc__, setFunc__) \
	private: \
		Type__ varName__; \
	public: \
		SETTER_GETTER(Type__, varName__, getFunc__, setFunc__)


/// Read only private property
///
/// - It deliberately does not work with pointers
/// - Dont use it with semicolon at the end (eg PROPERTY_RW(....);) because of a doxygen bug
#define PROPERTY_R(Type__, varName__, getFunc__) \
	private: \
		Type__ varName__; \
	public: \
		GETTER_R(Type__, varName__, getFunc__)


#endif
