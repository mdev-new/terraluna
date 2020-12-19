#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) {
	vec2 pos = ( gl_FragCoord.xy / resolution.xy ) - vec2(0.5,0.5);	
        float horizon = sin(time*1.8)/5.0; 
        float fov = 2.6; 
	float scaling = 0.1;
	
	vec3 p = vec3(pos.x-sin(time)/8.0, fov, abs(pos.y - horizon));      
	vec2 s = vec2(p.x/p.z, p.y/p.z) * scaling;
	
	// glitch texture
	float color = sign((mod(s.x+sin(time)/5.0, 0.1) - 0.05) * (mod(s.y+time*0.2, 0.1) - 0.05));	
	//fading
	color *= p.z*p.z*10.0;
	
	gl_FragColor = vec4( vec3(0.0,color*30.0,color), 1.0 )+1.0-color;

}