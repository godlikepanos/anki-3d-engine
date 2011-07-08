#ifndef S_PROG_LOADER_H
#define S_PROG_LOADER_H


namespace boost { namespace property_tree {
class ptree;
}}


class SProgLoader
{
	public:
		SProgLoader() {}
		SProgLoader(const char* filename) {parseFile(filename);}
		SProgLoader(const boost::property_tree::ptree& pt) {parsePtree(pt);}
		~SProgLoader();

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
};


#endif
