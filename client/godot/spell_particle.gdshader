shader_type canvas_item;
render_mode unshaded, blend_disabled;
uniform sampler2D background : hint_screen_texture, repeat_disable, filter_nearest;
uniform bool glow = false;
varying vec3 particle_tint;
void vertex() { particle_tint = COLOR.rgb; }
void fragment() {
    vec4 texel = texture(TEXTURE, UV);
    vec3 source = texel.rgb * particle_tint;
    vec3 destination = texture(background, SCREEN_UV).rgb;
    // 469290：ONE/INVSRCALPHA，材质透明度固定1/255。
    // 469CD0 -> 492788(2)：ONE/INVSRCCOLOR。
    vec3 factor = glow ? vec3(1.0)-source : vec3(1.0-texel.a/255.0);
    COLOR = vec4(clamp(source+destination*factor,vec3(0.0),vec3(1.0)),1.0);
}
