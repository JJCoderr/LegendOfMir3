shader_type canvas_item;
render_mode unshaded, blend_disabled;
// 原版492788模式2与49295D基线：ONE / INVSRCOLOR。
uniform sampler2D background : hint_screen_texture, repeat_disable, filter_nearest;
varying vec3 vertex_tint;
void vertex() { vertex_tint = COLOR.rgb; }
void fragment() {
    vec4 sample_color = texture(TEXTURE, UV);
    if (sample_color.a == 0.0) discard;
    vec3 source = sample_color.rgb * vertex_tint;
    vec3 destination = texture(background, SCREEN_UV).rgb;
    COLOR = vec4(clamp(source + destination * (vec3(1.0) - source), vec3(0.0), vec3(1.0)), 1.0);
}
