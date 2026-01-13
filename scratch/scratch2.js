function compile_program(vs_source, fs_source) {

	const vs = gl.createShader(gl.VERTEX_SHADER)
	gl.shaderSource(vs, vs_source)
	gl.compileShader(vs)
	if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS) > 0) {
		throw gl.getShaderInfoLog(vs)
	}

	const fs = gl.createShader(gl.FRAGMENT_SHADER)
	gl.shaderSource(fs, fs_source)
	gl.compileShader(fs)
	if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS) > 0) {
		throw gl.getShaderInfoLog(fs)
	}

	const program = gl.createProgram()
	gl.attachShader(program, vs)
	gl.attachShader(program, fs)

	gl.linkProgram(program)
	if (!gl.getProgramParameter(program, gl.LINK_STATUS) > 0) {
		throw gl.getProgramInfoLog(program)
	}
	return program

} 

function make_geom() {
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

	let vao = gl.createVertexArray()
	gl.bindVertexArray(vao)

	for (let i = 0; i < geometry.attributes.length; i += 1) {
		let buf = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, buf)
		let f32 = new Float32Array(geometry.attributes[i].flat())
		gl.bufferData(gl.ARRAY_BUFFER, f32, gl.STATIC_DRAW)
		gl.vertexAttribPointer(i, geometry.attributes[i][0].length, gl.FLOAT, false, 0, 0)
		gl.enableVertexAttribArray(i)
	}

	var indexbuf = gl.createBuffer()
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexbuf)
	let indexData = new Uint16Array(geometry.triangles.flat())
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indexData, gl.STATIC_DRAW)

	return {
		vao: vao,
		mode: gl.TRIANGLES,
		count: indexData.length,
		type: gl.UNSIGNED_SHORT,
		offset: 0
	}

}

function draw(ticks) {
	gl.clear(gl.COLOR_BUFFER_BIT)
	gl.useProgram(program)
	gl.bindVertexArray(geom.vao)

	var m = m4rotZ(Math.sin(ticks))
	gl.uniformMatrix4fv(program.uniforms.m, false, m)

	gl.drawElements(geom.mode, geom.count, geom.type, geom.offset)
	requestAnimationFrame(draw)
}

function getUniforms() {
	const uniforms = {}
	for (let i = 0; i < gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS); i += 1) {
		let info = gl.getActiveUniform(program, i)
		uniforms[info.name] = gl.getUniformLocation(program, info.name)
	}
	program.uniforms = uniforms
}

window.onload = (async (event) => {
	const fs_source = await fetch("fragment.glsl").then(res => res.text())
	const vs_source = await fetch("vertex.glsl").then(res => res.text())
	console.log(vs_source)
	console.log(fs_source)
	window.gl = document.getElementById("canvas").getContext("webgl2")
	window.program = compile_program(vs_source, fs_source)
	getUniforms()
	window.geom = make_geom()
	window.requestAnimationFrame(draw)
})
