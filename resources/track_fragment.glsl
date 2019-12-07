#version 330 core
out vec3 color;
//in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform vec3 campos;
uniform vec3 color_change;
uniform vec3 light_pos;

uniform sampler2D tex;
uniform int cart;
//uniform sampler2D tex2;

void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=vec3(0,50,0);
vec3 ld = normalize(light_pos - vertex_pos);
float diffuse = dot(n,ld);

if (cart == 0) {
	color.rgb = texture(tex, vertex_tex).rbg;
}
else {
	color.rgb = color_change + .1;
}
color.a = 1.f;
color.rgb *= diffuse*0.7;

vec3 cam = vec3(-campos.x, -campos.y, -campos.z);
vec3 cd = normalize(vertex_pos - cam);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,100);
color.rgb += vec3(1,1,1)*spec*3;
color.a=1;


}
