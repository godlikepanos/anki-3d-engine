R"(		<program>
			<type>frag</type>

			<includes>
				<include>shaders/MsCommonFrag.glsl</include>
			</includes>

			<inputs>
				<input><type>float</type><name>uSpecularPower</name><value>%specularPower%</value></input>
				<input><type>float</type><name>uBlurring</name><value>0.0</value><const>1</const></input>
				<input><type>sampler2D</type><name>uDiffuseMap</name><value>%diffuseMap%</value></input>
				<input><type>sampler2D</type><name>uNormalMap</name><value>%normalMap%</value></input>
				<input><type>sampler2D</type><name>uSpecularColorMap</name><value>%specularColorMap%</value></input>
			</inputs>
			
			<operations>
				<operation>
					<id>0</id>
					<returnType>vec3</returnType>
					<function>getNormal</function>
				</operation>
				<operation>
					<id>1</id>
					<returnType>vec4</returnType>
					<function>getTangent</function>
				</operation>
				<operation>
					<id>2</id>
					<returnType>vec2</returnType>
					<function>getTextureCoord</function>
				</operation>

				<operation>
					<id>10</id>
					<returnType>vec3</returnType>
					<function>readRgbFromTexture</function>
					<arguments>
						<argument>uDiffuseMap</argument>
						<argument>out2</argument>
					</arguments>
				</operation>
				<operation>
					<id>20</id>
					<returnType>vec3</returnType>
					<function>readNormalFromTexture</function>
					<arguments>
						<argument>out0</argument>
						<argument>out1</argument>
						<argument>uNormalMap</argument>
						<argument>out2</argument>
					</arguments>
				</operation>
				<operation>
					<id>30</id>
					<returnType>float</returnType>
					<function>readRFromTexture</function>
					<arguments>
						<argument>uSpecularColorMap</argument>
						<argument>out2</argument>
					</arguments>
				</operation>
				
				<operation>
					<id>40</id>
					<returnType>void</returnType>
					<function>writeRts</function>
					<arguments>
						<argument>out10</argument>
						<argument>out20</argument>
						<argument>out30</argument>
						<argument>uSpecularPower</argument>
						<argument>uBlurring</argument>
					</arguments>
				</operation>
			</operations>
		</program>)"

