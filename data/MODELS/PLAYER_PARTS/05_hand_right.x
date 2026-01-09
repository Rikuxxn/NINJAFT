xof 0302txt 0064
template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

Header{
1;
0;
1;
}

Mesh {
 18;
 -2.65002;-1.33818;0.93651;,
 0.47157;-1.57866;1.70975;,
 0.06802;0.64160;1.70975;,
 -2.93058;0.67867;1.29572;,
 -2.59823;1.68072;1.27414;,
 0.40036;1.64365;1.68817;,
 0.18121;1.52165;-1.73132;,
 -2.59823;1.68072;-1.31730;,
 -2.93058;0.67867;-1.29572;,
 -0.15112;0.51961;-1.70975;,
 0.64020;-1.57866;-1.70975;,
 -2.65002;-1.33818;-0.93651;,
 0.47157;-1.57866;1.70975;,
 -2.65002;-1.33818;0.93651;,
 0.64020;-1.57866;-1.70975;,
 -0.15112;0.51961;-1.70975;,
 -2.65002;-1.33818;-0.93651;,
 -2.93058;0.67867;-1.29572;;
 
 10;
 4;0,1,2,3;,
 4;4,5,6,7;,
 4;8,9,10,11;,
 4;11,10,12,13;,
 4;1,14,15,2;,
 4;16,0,3,17;,
 4;3,2,5,4;,
 4;2,9,6,5;,
 4;9,8,7,6;,
 4;8,3,4,7;;
 
 MeshMaterialList {
  1;
  10;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;;
  Material {
   0.351373;0.351373;0.351373;1.000000;;
   5.000000;
   0.000000;0.000000;0.000000;;
   0.000000;0.000000;0.000000;;
  }
 }
 MeshNormals {
  14;
  -0.190783;-0.118658;0.974434;,
  -0.163997;-0.026309;0.986110;,
  -0.171109;-0.057446;-0.983576;,
  -0.195117;-0.141494;-0.970520;,
  0.037297;0.999135;-0.018389;,
  -0.075278;-0.997161;-0.001818;,
  0.963632;0.266980;-0.011653;,
  0.999169;-0.025296;-0.031960;,
  -0.990463;-0.137781;0.000000;,
  -0.995861;0.090890;0.000000;,
  -0.135672;0.066287;0.988534;,
  0.947678;-0.315369;-0.049483;,
  -0.145765;0.027052;-0.988949;,
  -0.949156;0.314807;0.000000;;
  10;
  4;0,0,1,1;,
  4;4,4,4,4;,
  4;2,2,3,3;,
  4;5,5,5,5;,
  4;6,6,7,7;,
  4;8,8,9,9;,
  4;1,1,10,10;,
  4;7,7,11,11;,
  4;2,2,12,12;,
  4;9,9,13,13;;
 }
 MeshTextureCoords {
  18;
  0.375000;0.000000;,
  0.625000;0.000000;,
  0.625000;0.250000;,
  0.375000;0.250000;,
  0.375000;0.250000;,
  0.625000;0.250000;,
  0.625000;0.500000;,
  0.375000;0.500000;,
  0.375000;0.500000;,
  0.625000;0.500000;,
  0.625000;0.750000;,
  0.375000;0.750000;,
  0.625000;1.000000;,
  0.375000;1.000000;,
  0.875000;0.000000;,
  0.875000;0.250000;,
  0.125000;0.000000;,
  0.125000;0.250000;;
 }
}
