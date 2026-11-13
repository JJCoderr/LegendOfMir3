shader_type canvas_item;
render_mode unshaded, blend_disabled;
uniform sampler2D scene_color : hint_screen_texture, repeat_disable, filter_nearest;
uniform sampler2D dye_regions : repeat_disable, filter_nearest;
// Integer RGB565 channel values, quantized by the C++ state adapter.
uniform vec3 first_dye = vec3(0.0);
void fragment() {
    vec4 texel=texture(TEXTURE,UV);
    if (texel.a==0.0) discard;
    ivec3 steps=ivec3(8,4,8);
    ivec3 maximum=ivec3(31,63,31);
    ivec3 source=ivec3(round(texel.rgb*255.0))/steps;
    int region=int(round(texture(dye_regions,UV).r*255.0));
    if (region==2) source=(source*ivec3(first_dye))/maximum;
    // C3's second dye is 0xFFFF, so it leaves the source channels unchanged.
    ivec3 destination=ivec3(round(texture(scene_color,SCREEN_UV).rgb*255.0))/steps;
    ivec3 output_rgb=source*25/32+destination*7/32;
    COLOR=vec4(vec3(output_rgb*steps)/255.0,1.0);
}
