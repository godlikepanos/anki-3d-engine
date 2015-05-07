R"(		<program>
			<type>vert</type>
			<includes>
				<include>shaders/MsCommonVert.glsl</include>
			</includes>

			<inputs>
				<input><type>mat4</type><name>uMvp</name><value></value><instanced>%instanced%</instanced><arraySize>%arraySize%</arraySize></input>
				<input><type>mat3</type><name>uN</name><value></value><instanced>%instanced%</instanced><arraySize>%arraySize%</arraySize></input>
			</inputs>

			<operations>
				<operation>
					<id>1</id>
					<returnType>void</returnType>
					<function>writePositionNormalTangentTexCoord</function>
					<arguments><argument>uMvp</argument><argument>uN</argument></arguments>
				</operation>
			</operations>
		</program>

		<program>
			<type>tesc</type>
			<includes><include>shaders/MsCommonTessc.glsl</include></includes>

			<inputs>
				<input><type>float</type><name>uMaxTessLevel</name><value>12.0</value></input>
				<input><type>mat4</type><name>uMvp</name><value></value></input>
				<input><type>mat3</type><name>uN</name><value></value></input>
			</inputs>

			<operations>
				<operation>
					<id>0</id>
					<returnType>void</returnType>
					<function>tessellateDispMapPositionNormalTangentTexCoord</function>
					<arguments>
						<argument>uMaxTessLevel</argument>
						<argument>uMvp</argument>
						<argument>uN</argument>
					</arguments>
				</operation>
			</operations>
		</program>

		<program>
			<type>tese</type>
			<includes><include>shaders/MsCommonTesse.glsl</include></includes>

			<inputs>
				<input><type>mat4</type><name>uMvp</name><value></value></input>
				<input><type>mat3</type><name>uN</name><value></value></input>
				<input><type>sampler2D</type><name>dispMap</name><value>%dispMap%</value></input>
			</inputs>

			<operations>
				<operation>
					<id>0</id>
					<returnType>void</returnType>
					<function>tessellateDispMapPositionNormalTangentTexCoord</function>
					<arguments>
						<argument>uMvp</argument>
						<argument>uN</argument>
						<argument>dispMap</argument>
					</arguments>
				</operation>
			</operations>
		</program>)"
