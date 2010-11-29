#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Model.h"
#include "Material.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "MeshData.h"
#include "Vao.h"
#include "Skeleton.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


#define MDL_EXCEPTION(x) EXCEPTION("Model \"" + filename + "\": " + x)


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Model::load(const char* filename)
{
	//
	// Load
	//
	try
  {
		using namespace boost::property_tree;
		ptree pt_;
  	read_xml(filename, pt_);

  	ptree& pt = pt_.get_child("model");

  	// subModels
  	BOOST_FOREACH(ptree::value_type& v, pt.get_child("subModels"))
  	{
  		const std::string& mesh = v.second.get<std::string>("mesh");
  		const std::string& material = v.second.get<std::string>("material");
  		const std::string& dpMaterial = v.second.get<std::string>("dpMaterial");

  		subModels.push_back(SubModel());
  		subModels.back().load(mesh.c_str(), material.c_str(), dpMaterial.c_str());
  	}

  	// skeleton
  	boost::optional<std::string> skelName = pt.get_optional<std::string>("skeleton");
  	if(skelName.is_initialized())
  	{
  		skeleton.loadRsrc(skelName.get().c_str());
  	}


  	boost::optional<ptree&> skelAnims_ = pt.get_child_optional("skelAnims");
  	if(skelAnims_.is_initialized())
  	{
  		BOOST_FOREACH(ptree::value_type& v, skelAnims_.get())
  		{
  			const std::string& name = v.second.data();
  			skelAnims.push_back(RsrcPtr<SkelAnim>());
				skelAnims.back().loadRsrc(name.c_str());
  		}
  	}
	}
	catch(std::exception& e)
	{
		throw MDL_EXCEPTION(e.what());
	}


	//
	// Sanity checks
	//
	try
	{
		if(skelAnims.size() > 0 && !hasSkeleton())
		{
			throw EXCEPTION("You have skeleton animations but no skeleton");
		}

		for(uint i = 0; i < skelAnims.size(); i++)
		{
			// Bone number problem
			if(skelAnims[i]->bones.size() != skeleton->bones.size())
			{
				throw EXCEPTION("SkelAnim \"" + skelAnims[i]->getRsrcName() + "\" and Skeleton \"" +
				                skeleton->getRsrcName() + "\" dont have equal bone count");
			}
		}

		for(uint i = 0; i < subModels.size(); i++)
		{
			if(hasSkeleton())
			{
				if(!subModels[i].hasHwSkinning())
				{
					throw EXCEPTION("SubModel " + boost::lexical_cast<std::string>(i) + " material does not have HW skinning");
				}

				if(!subModels[i].hasHwSkinning())
				{
					throw EXCEPTION("SubModel " + boost::lexical_cast<std::string>(i) + " DP material does not have HW skinning");
				}
			}
		}
	}
	catch(std::exception& e)
	{
		throw MDL_EXCEPTION(e.what());
	}
}


//======================================================================================================================
// hasHwSkinning                                                                                                       =
//======================================================================================================================
bool Model::SubModel::hasHwSkinning() const
{
	return material->hasHwSkinning();
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Model::SubModel::load(const char* meshFName, const char* mtlFName, const char* dpMtlFName)
{
	//
	// Load
	//
	mesh.loadRsrc(meshFName);
	material.loadRsrc(mtlFName);
	dpMaterial.loadRsrc(dpMtlFName);

	//
	// Sanity checks
	//
	#define EXCEPTION_INCOMPATIBLE_RSRCS(x, y) \
		EXCEPTION("Resource \"" + x->getRsrcName() + "\" and \"" + y->getRsrcName() + "\" are incompatible")

	// if mtl needs tex coords then mesh should have
	if(material->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(material, mesh);
	}

	if(dpMaterial->hasTexCoords() && !mesh->hasTexCoords())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(dpMaterial, mesh);
	}

	// if mtl needs weights then mesh should have
	if(material->hasHwSkinning() && !mesh->hasVertWeights())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(material, mesh);
	}

	if(dpMaterial->hasHwSkinning() && !mesh->hasVertWeights())
	{
		throw EXCEPTION_INCOMPATIBLE_RSRCS(dpMaterial, mesh);
	}

	//
	// VAOs
	//
	createVao(*material, *mesh, *this, vao);
	createVao(*dpMaterial, *mesh, *this, dpVao);
}


//======================================================================================================================
// createVao                                                                                                           =
//======================================================================================================================
void Model::SubModel::createVao(const Material& mtl, const Mesh& mesh, SubModel& subModel, Vao*& vao)
{
	vao = new Vao(&subModel);

	if(mtl.getStdAttribVar(Material::SAV_POSITION) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_POSITIONS), *mtl.getStdAttribVar(Material::SAV_POSITION),
		                          3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_NORMAL) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_NORMALS), *mtl.getStdAttribVar(Material::SAV_NORMAL),
		                          3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TANGENT) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_TANGENTS), *mtl.getStdAttribVar(Material::SAV_TANGENT),
		                          4, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_TEX_COORDS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_TEX_COORDS), *mtl.getStdAttribVar(Material::SAV_TEX_COORDS),
		                          2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONES_NUM), 1,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(0));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_BONE_IDS), 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(4));
	}

	if(mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS) != NULL)
	{
		vao->attachArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_WEIGHTS),
		                          *mtl.getStdAttribVar(Material::SAV_VERT_WEIGHT_WEIGHTS), 4,
		                          GL_FLOAT, GL_FALSE, sizeof(MeshData::VertexWeight), BUFFER_OFFSET(20));
	}

	vao->attachElementArrayBufferVbo(*mesh.getVbo(Mesh::VBO_VERT_INDECES));
}
