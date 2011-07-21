#ifndef S_PROG_LOADER_H
#define S_PROG_LOADER_H

#include "Util/Vec.h"
#include "Util/Accessors.h"
#include <string>
#include <boost/property_tree/ptree_fwd.hpp>


/// @todo
class SProgLoader
{
	public:
		SProgLoader() {}
		SProgLoader(const char* filename) {parseFile(filename);}
		SProgLoader(const boost::property_tree::ptree& pt) {parsePtree(pt);}
		~SProgLoader();

		/// @name Accessors
		/// @{
		GETTER_R(Vec<std::string>, out.tfbVaryings, getTranformFeedbackVaryings)
		GETTER_R(std::string, out.vertShaderSource, getVertexShaderSource)
		GETTER_R(std::string, out.geomShaderSource, getGeometryShaderSource)
		GETTER_R(std::string, out.fragShaderSource, getFragmentShaderSource)
		/// @}

		void parseFile(const char* filename);
		void parsePtree(const boost::property_tree::ptree& pt);

	private:
		struct Output
		{
			Vec<std::string> tfbVaryings; ///< Names and and ids for transform
			                              ///< feedback varyings
			std::string vertShaderSource; ///< The vert shader source
			std::string geomShaderSource; ///< The geom shader source
			std::string fragShaderSource; ///< The frag shader source
		};

		Output out;

		/// Parse what is inside the:
		/// @code <shaderProgram>...</shaderProgram> @endcode
		static void parseShaderProgBlock(const boost::property_tree::ptree& pt,
			Output& out);

		/// Parse what is inside a shader block. It appends the src string
		static void parseShaderBlock(const boost::property_tree::ptree& pt,
			std::string& src);
};


#endif
