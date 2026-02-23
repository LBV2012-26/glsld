#version 460

void Func(int[3][2] mdi, int);
void Func(float[3][2] mdf, int);


void main() {
    int md[][] = int[][](int[](1, 2), int[](3, 4), int[](5, 6));

    int mmd[][][] = int[][][](int[][](int[](1, 2), int[](3, 4), int[](5, 6)),
                              int[][](int[](1, 2), int[](3, 4), int[](5, 6)),
                              int[][](int[](1, 2), int[](3, 4), int[](5, 6)),
                              int[][](int[](1, 2), int[](3, 4), int[](5, 6)))

    Func(md, 0);
}