#ifndef ACCESSORS_H
#define ACCESSORS_H


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
	void setter__(const type__ x__) {var__ = x__;}


/// The macro implies read write var
#define GETTER_SETTER(type__, var__, getter__, setter__) \
	GETTER_RW(type__, var__, getter__) \
	SETTER(type__, var__, setter__)


/// The macro implies read write var
#define GETTER_SETTER_BY_VAL(type__, var__, getter__, setter__) \
	GETTER_RW_BY_VAL(type__, var__, getter__) \
	SETTER_BY_VAL(type__, var__, setter__)


#endif
