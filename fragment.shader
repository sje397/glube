#version 120

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

uniform vec3 eyePosition;
uniform float clock;

varying vec3 position;

void main(){
    float distance = length(position - eyePosition);

    gl_FragColor = vec4(0, 0, 1.0, 1);

    float fog_start = 150, fog_end = 200;
    float dsq = distance * distance;
    if(dsq > fog_start * fog_start) {
        if(dsq > fog_end * fog_end)
            dsq = fog_end * fog_end;
        float g = (dsq - fog_start * fog_start) / (fog_end * fog_end - fog_start * fog_start);
        gl_FragColor = vec4((1 - g) * vec3(0.5, 1.0, 0.5), 1.0) + vec4(g * vec3(135/255.0, 196/255.0, 250/255.0) * clock, 0.0);
        //gl_FragColor *= 0;
    }
}

