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
		</program>)"
