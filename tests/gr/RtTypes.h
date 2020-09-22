MAGIC_MACRO(
	struct Material {
		Vec3 m_diffuseColor;
		Vec3 m_emissiveColor;
	};

	struct Mesh {
		U64 m_indexBufferPtr;
		U64 m_positionBufferPtr;
	};

	struct Model {
		Material m_mtl;
		Mesh m_mesh;
	};

	struct Light {
		Vec3 m_min;
		Vec3 m_max;
		Vec3 m_intensity;
	};

	struct PushConstants {
		Mat4 m_vp;
		Vec3 m_cameraPos;
		U32 m_lightCount;
		UVec3 m_padding0;
		U32 m_frame;
	};)
