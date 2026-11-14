shader_type canvas_item;
render_mode unshaded, blend_disabled;
// 492788模式分派；49295D恢复源ONE、目标ZERO及普通纹理乘色。
uniform sampler2D background : hint_screen_texture, repeat_disable, filter_nearest;
uniform int effect_blend = 2;
uniform float effect_opacity = 1.0;
varying vec3 vertex_tint;
void vertex() { vertex_tint = COLOR.rgb; }
void fragment() {
    vec4 sample_color = texture(TEXTURE, UV);
    if (sample_color.a == 0.0) discard;
    vec3 source = sample_color.rgb * vertex_tint;
    vec3 destination = texture(background, SCREEN_UV).rgb;
    vec3 result = source + destination * (vec3(1.0) - source);
    if (effect_blend == 0) {
        // 源ONE，目标INVSRCALPHA；透明度只衰减背景，不再乘一次源颜色。
        result = source + destination * (1.0 - sample_color.a * effect_opacity);
    } else if (effect_blend == 1) {
        result = source + destination;
    } else if (effect_blend == 3) {
        source = (vec3(1.0) - sample_color.rgb) * vertex_tint;
        if (effect_opacity >= 1.0) {
            result = source * (1.0 - sample_color.a);
        } else {
            result = source + destination * (1.0 - sample_color.a * effect_opacity);
        }
    } else if (effect_blend == 4 || effect_blend == 5) {
        // 49280A与492812均把COLORARG1设为TEXTURE|COMPLEMENT。
        source = (vec3(1.0) - sample_color.rgb) * vertex_tint;
        result = source + destination * (vec3(1.0) - destination);
    } else if (effect_blend == 6) {
        result = destination * source;
    }
    COLOR = vec4(clamp(result, vec3(0.0), vec3(1.0)), 1.0);
}
