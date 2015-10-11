R"(		<program>
			<type>vert</type>
			<includes>
				<include>shaders/MsCommonVert.glsl</include>
			</includes>

			<inputs>
				<input><type>mat4</type><name>anki_mvp</name></input>
				<input><type>mat3</type><name>anki_n</name><inShadow>0</inShadow></input>
			</inputs>

			<operations>
				<operation>
					<id>0</id>
					<returnType>void</returnType>
					<function>writePositionAndUv</function>
					<arguments><argument>anki_mvp</argument></arguments>
				</operation>
				<operation>
					<id>1</id>
					<returnType>void</returnType>
					<function>writeNormalAndTangent</function>
					<arguments><argument>anki_n</argument></arguments>
				</operation>
			</operations>
		</program>)"
