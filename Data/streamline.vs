
uniform vec2 sizes;
uniform sampler2D texPos;

uniform float osg_SimulationTime;
uniform float totalInstances;
uniform float fadeTime;
uniform float repeatTime;

void main()
{
    // Using the instance ID, generate "texture coords" for this instance.
    const float r = ((float)gl_InstanceID) / sizes.x;
    vec2 tC;
    tC.s = fract( r ); tC.t = floor( r ) / sizes.y;

    // Get position from the texture.
    vec4 pos = texture2D( texPos, tC );
    pos.w = 0.; // w is 1.0 after lookup; do NOT add 1.0 to gl_Vertex.w
    vec4 v = gl_ModelViewMatrix * ( gl_Vertex + pos );
    gl_Position = gl_ProjectionMatrix * v;

    // TBD. Need to make this configurable from a uniform.
    gl_PointSize = -2000. / v.z;

    // Compute a time offset from the InstanceID to
    // emulate motion.
    float timeOffset = ( ((float)gl_InstanceID) / totalInstances ) * repeatTime;
    float repTimer = mod( ( osg_SimulationTime - timeOffset ), repeatTime );
    // Normalize by fadetime so that we actually get a long smooth
    // alpha gradient across all of the streamline
    float alpha = (fadeTime - min( repTimer, fadeTime ))/fadeTime;
    vec4 color = gl_Color;
    color.a *= alpha;
    gl_FrontColor = color;
}
