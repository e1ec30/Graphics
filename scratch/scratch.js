function compileShader(vs_source, fs_source) {
	const vs = gl.createShader(gl.VERTEX_SHADER)
	gl.shaderSource(vs, vs_source)
	gl.compileShader(vs)

	if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
		console.error(gl.getShaderInfoLog(vs))
		throw Error("Vertex Shader couldn't compile")
	}

	const fs = gl.createShader(gl.FRAGMENT_SHADER)
	gl.shaderSource(fs, fs_source)
	gl.compileShader(fs)

	if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
		console.error(gl.getShaderInfoLog(fs))
		throw Error("Fragment Shader couldn't compile")
	}

	const program = gl.createProgram();
	gl.attachShader(program, vs)
	gl.attachShader(program, fs)
	gl.linkProgram(program)

	if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
		console.error(gl.getProgramInfoLog(program))
		throw Error("Couldn't link")
	}

	return program

}

function setupGeometry() {
	geometry = {
		"triangles": [0, 1, 2],
		"attributes": 
			[[
				[0.0, 0.5]
				, [-0.5, -0.5]
				, [0.5, -0.5]
			],
			[
				[0.0, 0.0, 1.0, 1.0]
				, [1.0, 0.0, 0.0, 1.0]
				, [0.0, 1.0, 0.0, 1.0]
			]]
	};
	let triangleVertexArray = gl.createVertexArray()
	gl.bindVertexArray(triangleVertexArray)

	for (let i = 0; i < geometry.attributes.length; i += 1) {

		let buf = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, buf)

		let f32 = new Float32Array(geometry.attributes[i].flat())

		gl.bufferData(gl.ARRAY_BUFFER, f32, gl.STATIC_DRAW)
		
		gl.vertexAttribPointer(i, geometry.attributes[i][0].length, gl.FLOAT, false, 0, 0)

		gl.enableVertexAttribArray(i)
	}

	var indices = new Uint16Array(geometry.triangles.flat())

	var indexBuffer = gl.createBuffer()
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer)
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW)

	return {
		mode: gl.TRIANGLES,
		count: indices.length,
		type: gl.UNSIGNED_SHORT,
		vao: triangleVertexArray
	}
}

function getUniforms() {
	const uniforms = {}
	for (let i = 0; i < gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS); i += 1) {
		let info = gl.getActiveUniform(program, i)
		uniforms[info.name] = gl.getUniformLocation(program, info.name)
	}
	program.uniforms = uniforms
}

function draw() {
	gl.clear(gl.COLOR_BUFFER_BIT)
	gl.useProgram(program)
	gl.bindVertexArray(geom.vao)
	gl.drawElements(geom.mode, geom.count, geom.type, 0)
}

window.addEventListener('load', async () => {
	window.gl = document.querySelector("canvas").getContext('webgl2')
	fs = await fetch("fragment.glsl").then(res => res.text())
	vs = await fetch("vertex.glsl").then(res => res.text())
	window.program = compileShader(vs, fs)
	window.geom = setupGeometry()
	getUniforms()
	draw()
})
