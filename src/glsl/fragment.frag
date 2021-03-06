#version 330 core
in vec2 TexCoord;

out vec4 FragColor;
layout(location = 0) out vec4 color_out;

uniform sampler2D texture_u;
uniform vec2 mouse_pos_u;
uniform int mouse_leftButton_u;
uniform int selected_material;

vec4 getTexColor(vec2 off);
vec4 updatePixel();
bool equalFloat(float a, float b);
bool eqCol(vec4 color1, vec4 color2);
bool neqCol(vec4 color1, vec4 color2);
vec4 gen_circle();
vec4 matColor();

float width = 1920;
float height = 1080;
int width_as_int= int(width);
int height_as_int = int(height);

vec4 fake_color = vec4(-1.0,0.0,0.0,0.0);
vec4 outside = vec4(-2.0,0.0,0.0,0.0);
vec4 black = vec4(0.0,0.0,0.0,0.0);
vec4 magenta = vec4(1.0,0.0,1.0,1.0);
vec4 sand_color = vec4(0.77,0.52,0.0,1.0);
vec4 wood_color = vec4(0.15,0.80,0.2,1.0);
vec4 water_color = vec4(0.05,0.0,0.9,1.0);
vec2 unitPixel = vec2(1.0,1.0)/vec2(width,height);

vec2 up = vec2(0.0,1.0);
vec2 down = vec2(0.0,-1.0);
vec2 left = vec2(-1.0,0.0);
vec2 right = vec2(1.0,0.0);
vec2 here = vec2(0.0,0.0);

vec4 upColor = getTexColor(up);
vec4 downColor = getTexColor(down);
vec4 hereColor = getTexColor(here);
vec4 leftColor = getTexColor(left);
vec4 leftleftColor = getTexColor(left+left);
vec4 leftleftleftColor = getTexColor(left+left+left);
vec4 rightColor = getTexColor(right);
vec4 rightrightColor = getTexColor(right+right);
vec4 downleftColor = getTexColor(down+left);
vec4 downleftleftColor = getTexColor(down+left+left);
vec4 downrightColor = getTexColor(down+right);
vec4 downrightrightColor = getTexColor(down+right+right);
vec4 upleftColor = getTexColor(up+left);
vec4 upleftleftColor = getTexColor(up+left+left);
vec4 uprightColor = getTexColor(up+right);

vec2 pos = gl_FragCoord.xy/vec2(width,height);

// Gold Noise ©2015 dcerisano@standard3d.com
// - based on the Golden Ratio
// - uniform normalized distribution
// - fastest static noise generator function (also runs at low precision)
// - use with indicated seeding method. 
float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio   
float gold_noise(in vec2 xy, in float seed){
       return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

//circle arround the mouse
float isIn = step(sqrt(pow((mouse_pos_u.x-pos.x)*width,2)+pow((mouse_pos_u.y-pos.y)*height,2)),20);
//if the mouse is clicked, draws particles arround the circle
vec4 clicked_circle = gen_circle();

void main()
{
    vec4 res = updatePixel();
    FragColor = res;
    color_out = res;
}

bool eqCol(vec4 color1, vec4 color2){
    return (
            equalFloat(color1.x,color2.x) && 
            equalFloat(color1.y,color2.y) && 
            equalFloat(color1.z,color2.z)
           );
}
bool neqCol(vec4 color1, vec4 color2){ return !eqCol(color1,color2); }
bool equalFloat(float a, float b){ return abs(a-b) < 0.01; }
vec4 gen_circle(){ return isIn * mouse_leftButton_u * matColor(); }

vec4 matColor(){
    if(selected_material == 0) return black;
    if(selected_material == 1) return sand_color;
    if(selected_material == 2) return wood_color;
    if(selected_material == 3) return water_color;
    return magenta;
}

bool solid(vec4 mat){
    return 
        eqCol(mat, sand_color) ||  
        eqCol(mat, wood_color) || 
        // TODO flags? 
        eqCol(mat, water_color) 
        ;
}

bool falls(vec4 mat){
    return 
        eqCol(mat, sand_color) ||  
        eqCol(mat, water_color) 
        ;
}



bool inBound(vec2 direction){
    if( (pos.x + direction.x * unitPixel.x) >= 1.0) return false;
    if( (pos.y + direction.y * unitPixel.y) >= 1.0) return false;
    if( (pos.x + direction.x * unitPixel.x) < 0.0) return false;
    if( (pos.y + direction.y * unitPixel.y) < 0.0) return false;
    return true;
}

vec4 updateInBounds(vec4 mat){
    if(pos.y <= unitPixel.y && eqCol(hereColor,mat)) return hereColor;
    if(pos.x <= unitPixel.x && eqCol(upColor,mat)) return upColor;
    if(pos.x <= unitPixel.x && eqCol(uprightColor,mat)) return uprightColor;
    if(pos.x >= (unitPixel.x * width) && eqCol(upColor,mat)) return upColor;
    if(pos.x >= (unitPixel.x * (width-1)) && eqCol(upleftColor,mat)) return upleftColor;
    return fake_color;
}

vec4 updateFalling(vec4 mat){
    // gravity
    if(eqCol(hereColor,black) && eqCol(upColor,mat)) return upColor;
    if(eqCol(hereColor,mat) && eqCol(downColor,black)) return black;
    return fake_color;
}

bool canFallRight(vec4 mat){
    return (
            eqCol(downleftColor,mat) && 
            eqCol(rightColor,black) && 
            eqCol(downrightColor,black) && 
            eqCol(rightrightColor,black) &&
            eqCol(upColor,black) && 
            eqCol(uprightColor,black)
           );
}

bool canFallLeft(vec4 mat){
    return (
            eqCol(downrightColor,mat) && 
            eqCol(leftColor,black) && 
            eqCol(downleftColor,black) && 
            eqCol(leftleftColor,black) && 
            eqCol(upColor,black) && 
            eqCol(upleftColor,black)
           );
}

bool shouldLeftSlideInto(vec4 mat){
    return (
            eqCol(hereColor,black) && 
            eqCol(leftColor,mat) && 
            eqCol(downleftColor,mat) && 
            eqCol(downleftleftColor,mat) && 
            eqCol(rightColor,black) && 
            eqCol(upColor,black) && 
            eqCol(upleftColor,black) && 
            eqCol(downColor,black)
           );
}

bool shouldRightSlideInto(vec4 mat){
    return (
            eqCol(hereColor,black) && 
            eqCol(rightColor,mat) && 
            eqCol(downrightColor,mat) && 
            eqCol(downrightrightColor,mat) && 
            eqCol(leftColor,black) && 
            eqCol(upColor,black) && 
            eqCol(uprightColor,black) && 
            eqCol(downColor,black)
           );
}

vec4 updateSlide(vec4 mat, int d){
    // We are sliding
    if(eqCol(hereColor,mat) && solid(downColor)){
        if(canFallRight(mat)) { return black;}
        else if(canFallLeft(mat)){ return black;}
        else return hereColor;
    }
    // We are being slid into 
    if(shouldLeftSlideInto(mat)) return leftColor;
    if(shouldRightSlideInto(mat)) return rightColor;

    // already handled distance of 1 
    if(d <= 1) return fake_color;

    // TODO seed
    bool dir = gold_noise(vec2(pos.x, 0), 0) > 0.5;
    int dst = width_as_int/2;
    if(!dir) dst *= -1;
    int step = 1;

    while(step < width_as_int/2){
        step++;
    }



    return fake_color;
}

vec4 updateSand(){
    vec4 checker = fake_color;
    // if we are against a wall, just stay still
    checker = updateInBounds(sand_color);
    if(neqCol(checker, fake_color)) return checker;
    // if there is a sand above us, it should fall down 
    checker = updateFalling(sand_color);
    if(neqCol(checker, fake_color)) return checker;
    // we are sliding down 
    checker = updateSlide(sand_color, 1);
    if(neqCol(checker, fake_color)) return checker;
    return fake_color;
}

vec4 updateWater(){
    vec4 checker = fake_color;
    // if we are against a wall, just stay still
    checker = updateInBounds(water_color);
    if(neqCol(checker, fake_color)) return checker;
    checker = updateFalling(water_color);
    if(neqCol(checker, fake_color)) return checker;
    checker = updateSlide(water_color, int(width) );
    if(neqCol(checker, fake_color)) return checker;
    return fake_color;
}


vec4 updatePixel(){
    if( equalFloat(pos.y, 1.0) && equalFloat(pos.x, 0.25)) return sand_color;
    if( equalFloat(pos.y, 1.0) && equalFloat(pos.x, 0.75)) return water_color;
    if( equalFloat(pos.y, pos.x)) return wood_color;
    // if you press mouse, and we are in screen, write selectedMaterial
    if(isIn * mouse_leftButton_u == 1) return matColor();

    vec4 c = fake_color;
    if(eqCol(wood_color,hereColor)) return wood_color;

    // sand 
    c = updateSand();
    if(neqCol(c, fake_color)) return c;
    // water 
    c = updateWater();
    if(neqCol(c, fake_color)) return c;

    return clicked_circle;
}


vec4 getTexColor(vec2 off){
    if(TexCoord.y + unitPixel.y > height+1) return black;
    if(TexCoord.y + unitPixel.y < -1) return black;
    return texture(
            texture_u, 
            vec2(TexCoord.x + unitPixel.x*off.x,
                TexCoord.y + unitPixel.y*off.y
                )
            );
}
