// NOTE "#version xxx" inserted by host code.


// 'normal' must be normalized.
vec4 lighting( in vec4 ambProd, in vec4 diffProd, in vec4 specProd, in float specExp,
    in vec4 ecVertex, in vec3 normal, in vec3 lightVec )
{
    float diffDot = dot( normal, lightVec );
    vec4 diff = diffProd * max( diffDot, 0. );
   
    vec4 spec = vec4( 0., 0., 0., 0. );
    if( ( specExp > 0. ) && ( diffDot > 0. ) )
    {
        vec3 vertVec = normalize( ecVertex.xyz );
        vec3 reflectVec = reflect( lightVec, normal ); // lightVec and normal are already normalized,
        spec = specProd * pow( max( dot( reflectVec, vertVec ), 0. ), specExp );
    }

    return( ambProd + diff + spec );
}


uniform bool twoSided;


void main( void )
{
    gl_Position = ftransform();
    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );

    vec4 ec = gl_ModelViewMatrix * gl_Vertex;
    gl_ClipVertex = ec;

    vec3 lightVec = gl_LightSource[0].position.xyz;
    if( gl_LightSource[0].position.w > 0. )
        lightVec = normalize( lightVec - ec.xyz );

    gl_FrontColor = lighting( gl_FrontLightProduct[0].ambient + gl_LightModel.ambient,
        gl_FrontLightProduct[0].diffuse, gl_FrontLightProduct[0].specular,
        gl_FrontMaterial.shininess, ec, normal, lightVec );

    if( twoSided )
        gl_BackColor = lighting( gl_BackLightProduct[0].ambient + gl_LightModel.ambient,
            gl_BackLightProduct[0].diffuse, gl_BackLightProduct[0].specular,
            gl_BackMaterial.shininess, ec, -normal, lightVec );
}
