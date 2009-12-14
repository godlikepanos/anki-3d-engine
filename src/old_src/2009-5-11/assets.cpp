#include "assets.h"


namespace ass {


/**
=======================================================================================================================================
DATA OBJECTS                                                                                                                          =
=======================================================================================================================================
*/
static vector<data_class_t*> assets; // ToDo: update the funcs

static vector<texture_t*> textures;
static vector<shader_prog_t*> shaders;
static vector<material_t*> materials;
static vector<mesh_data_t*> mesh_data;
static vector<model_data_t*> model_data;


/**
=======================================================================================================================================
TEMPLATE FUNCS                                                                                                                        =
=======================================================================================================================================
*/

// Search (by name)
template <typename type_t> type_t* Search( vector<type_t*>& vec, const char* name )
{
	typename vector<type_t*>::iterator it;
	for( it=vec.begin(); it<vec.end(); it++ )
	{
		if( strcmp( name, (*it)->GetName() ) == 0 )
			return *it;
	}
	return NULL;
}

// Search (by pointer)
template <typename type_t> bool Search( vector<type_t*>& vec, type_t* x )
{
	typename vector<type_t*>::iterator it;
	for( it=vec.begin(); it<vec.end(); it++ )
	{
		if( x == *it )
			return true;
	}
	return false;
}

// Register
template <typename type_t> void Register( vector<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) ); // the obj must not be allready loaded

	x->SetID( vec.size() );
	vec.push_back( x );
}

// UnRegister
template <typename type_t> void UnRegister( vector<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) );

	// decr the ids
	typename vector<type_t*>::iterator it;
	for( it=vec.begin()+x->GetID()+1; it<vec.end(); it++ )
	{
		(*it)->SetID( (*it)->GetID()-1 );
	}

	vec.erase( vec.begin() + x->GetID() );
}

// LoadDataClass
template <typename type_t> type_t* LoadDataClass( vector<type_t*>& vec, const char* fname )
{
	char* name = CutPath(fname);
	type_t* newt = Search<type_t>( vec, name );

	// allready loaded
	if( newt != NULL )
	{
		++newt->users_num;
		return newt;
	}

	// not loaded
	newt = new type_t;
	newt->SetName( name );
	if( !newt->Load( fname ) )
	{
		ERROR( "Cannot load \"" << fname << '\"' );
		return NULL;
	}
	newt->users_num = 1;
	Register<type_t>( vec, newt );
	return newt;
}

// UnloadDataClass
template <typename type_t> void UnloadDataClass( vector<type_t*>& vec, type_t* x )
{
	DEBUG_ERR( Search<type_t>( vec, x ) );
	DEBUG_ERR( x->users_num < 1 );

	--x->users_num;

	if( x->users_num == 0 )
	{
		UnRegister<type_t>( vec, x );
		delete x;
	}
}


/*
=======================================================================================================================================
texture                                                                                                                               =
=======================================================================================================================================
*/
texture_t* SearchTxtr( const char* name )
{
	return Search<texture_t>( textures, name );
}

texture_t* LoadTxtr( const char* name )
{
	return LoadDataClass<texture_t>( textures, name );
}

void UnLoadTxtr( texture_t* txtr )
{
	UnloadDataClass<texture_t>( textures, txtr );
}


/*
=======================================================================================================================================
material                                                                                                                              =
=======================================================================================================================================
*/
material_t* SearchMat( const char* name )
{
	return Search<material_t>( materials, name );
}

material_t* LoadMat( const char* name )
{
	return LoadDataClass<material_t>( materials, name );
}

void UnLoadMat( material_t* mat )
{
	UnloadDataClass<material_t>( materials, mat );
}


/*
=======================================================================================================================================
shaders                                                                                                                               =
=======================================================================================================================================
*/
shader_prog_t* SearchShdr( const char* name )
{
	return Search<shader_prog_t>( shaders, name );
}

shader_prog_t* LoadShdr( const char* name )
{
	return LoadDataClass<shader_prog_t>( shaders, name );
}

void UnLoadMat( shader_prog_t* x )
{
	UnloadDataClass<shader_prog_t>( shaders, x );
}


/*
=======================================================================================================================================
mesh data                                                                                                                             =
=======================================================================================================================================
*/
mesh_data_t* SearchMeshD( const char* name )
{
	return Search<mesh_data_t>( mesh_data, name );
}

mesh_data_t* LoadMeshD( const char* name )
{
	return LoadDataClass<mesh_data_t>( mesh_data, name );
}

void UnLoadMeshD( mesh_data_t* x )
{
	UnloadDataClass<mesh_data_t>( mesh_data, x );
}


/*
=======================================================================================================================================
model data                                                                                                                            =
=======================================================================================================================================
*/
model_data_t* SearchModelD( const char* name )
{
	return Search<model_data_t>( model_data, name );
}

model_data_t* LoadModelD( const char* name )
{
	return LoadDataClass<model_data_t>( model_data, name );
}

void UnLoadModelD( model_data_t* x )
{
	UnloadDataClass<model_data_t>( model_data, x );
}


} // end namespace
