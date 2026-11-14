shader_type canvas_item;
render_mode unshaded, blend_disabled;

uniform sampler2D background : hint_screen_texture, repeat_disable, filter_nearest;
uniform float source_weight = 13.0;

void fragment() {
    vec4 source = texture(TEXTURE, UV);
    if (source.a == 0.0) discard;
    vec3 destination = texture(background, SCREEN_UV).rgb;
    vec3 channel_step = vec3(8.0, 4.0, 8.0);
    vec3 src565 = floor(round(source.rgb * 255.0) / channel_step);
    vec3 dst565 = floor(round(destination * 255.0) / channel_step);
    vec3 mixed = floor(src565 * source_weight / 32.0)
        + floor(dst565 * (32.0 - source_weight) / 32.0);
    COLOR = vec4(mixed * channel_step / 255.0, 1.0);
}
