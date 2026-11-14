shader_type canvas_item;
render_mode unshaded, blend_disabled;

uniform sampler2D background : hint_screen_texture, repeat_disable, filter_nearest;
uniform float source_weight = 13.0;
uniform bool combine_before_floor = false;
uniform bool highlight_opaque = false;

void fragment() {
    vec4 source = texture(TEXTURE, UV);
    if (source.a == 0.0) discard;
    vec3 destination = texture(background, SCREEN_UV).rgb;
    vec3 channel_step = vec3(8.0, 4.0, 8.0);
    vec3 src565 = floor(round(source.rgb * 255.0) / channel_step);
    vec3 dst565 = floor(round(destination * 255.0) / channel_step);
    vec3 mixed = floor(src565 * source_weight / 32.0)
        + floor(dst565 * (32.0 - source_weight) / 32.0);
    // 490821的状态色过渡使用整数(sum/2)，不同于48EF15分别截断。
    if (combine_before_floor) mixed = floor((src565 + dst565) / 2.0);
    // 48FA0F increases each source channel by 1.5 * channel² / maximum.
    vec3 maximum = vec3(31.0, 63.0, 31.0);
    if (highlight_opaque) mixed = min(maximum, floor(src565 + 1.5 * src565 * src565 / maximum));
    COLOR = vec4(mixed * channel_step / 255.0, 1.0);
}
