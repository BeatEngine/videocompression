#include <vector>
#include <stdlib.h>
#include <string.h>

enum class FrameType : char {
     FULL = 'F',
     RECT = 'R',
     PIXELS = 'P',
};

#define alloc(size) (unsigned char*)malloc(size);
#define alloi(size) (int*)malloc(size);

class Frame
{
    FrameType type = FrameType::FULL;
    unsigned char* pixels = 0;
    int* updatePositions = 0;
    int w = 0;
    int h = 0;
    int x = 0;
    int y = 0;
    public:

    void setX(int X)
    {
        x = X;
    }
    void setY(int Y)
    {
        y = Y;
    }
    void setW(int W)
    {
        w = W;
    }
    void setH(int H)
    {
        h = H;
    }
    void setType(FrameType typ)
    {
        type = typ;
    }
    void setData(unsigned char* pix, int width, int height)
    {
        *this = Frame(pix, width, height);
    }

    int getX()
    {
        return x;
    }

    int getY()
    {
        return y;
    }

    int* getUpdatePositions()
    {
        return updatePositions;
    }

    unsigned char* rawData()
    {
        return pixels;
    }

    FrameType getType()
    {
        return type;
    }

    int getWidth()
    {
        return w;
    }

    int getHeight()
    {
        return h;
    }

    Frame()
    {

    }

    Frame(const Frame& other)
    {
        w = other.w;
        h = other.h;
        type = other.type;
        x = other.x;
        y = other.y;
        if(other.pixels)
        {
            pixels = alloc(w*h*3);
            memcpy(pixels, other.pixels, w*h*3);
        }
        if(other.updatePositions)
        {
            updatePositions = alloi(w*h);
            memcpy(updatePositions, other.updatePositions, w*h);
        }
    }

    /*const Frame&*/void operator=(const Frame& other)
    {
        w = other.w;
        h = other.h;
        type = other.type;
        x = other.x;
        y = other.y;
        if(other.pixels)
        {
            pixels = alloc(w*h*3);
            memcpy(pixels, other.pixels, w*h*3);
        }
        if(other.updatePositions)
        {
            updatePositions = alloi(w*h);
            memcpy(updatePositions, other.updatePositions, w*h);
        }
        //return *this;
    }

    Frame(unsigned char* pixelsRGB, int width, int height)
    {
        pixels = alloc(width*height*3);
        memcpy(pixels, pixelsRGB, width*height*3);
        w = width;
        h = height;
    }

    void updateByFrame(Frame& next)
    {
        if(next.type == FrameType::FULL)
        {
            *this = next;
        }
        else if(next.getType() == FrameType::RECT)
        {
            int x;
            unsigned char* source = next.rawData();
            if(source)
            {
                int rx = 0;
                int ry = 0;
                for(int y = next.getY(); y < next.getHeight(); y++)
                {
                    rx = 0;
                    for(x = next.getX(); x < next.getWidth(); x++)
                    {
                        pixels[y*w*3+x*3] = source[ry*next.getWidth()*3+rx*3];
                        pixels[y*w*3+x*3+1] = source[ry*next.getWidth()*3+rx*3+1];
                        pixels[y*w*3+x*3+2] = source[ry*next.getWidth()*3+rx*3+2];
                        rx++;
                    }
                    ry++; 
                }
            }
        }
        else
        {
            unsigned char* source = next.rawData();
            int* pos = next.getUpdatePositions();
            if(source && pos)
            {
                int size = next.getWidth()*next.getHeight();

                for(int i = 0; i < size; i++)
                {
                    pixels[pos[i]] = source[i];
                    pixels[pos[i]+1] = source[i+1];
                    pixels[pos[i]+2] = source[i+2];
                }
            }
        }
    }

    ~Frame()
    {
        if(pixels)
        {
            free(pixels);
        }
        if(updatePositions)
        {
            free(updatePositions);
        }
    }

};

struct point
{
    int x = 0;
    int y = 0;
};

class VideoCompressor
{

    int width = 1920;
    int height = 1080;
    std::vector<Frame> frames;
    Frame current;

    public:

    VideoCompressor(int width = 1920, int height = 1080)
    {
        this->height = height;
        this->width = width;
    }

    void loadNextFrame(unsigned char* data)
    {
        if(frames.size() == 0)
        {
            frames.push_back(Frame(data, width, height));
            current = Frame(data, width, height);
        }
        else
        {
            std::vector<point> differences;
            int x;
            for(int y = 0; y < height; y++)
            {   
                for(x = 0; x < width; x++)
                {
                    if(current.rawData()[y*width*3+x*3] != data[y*width*3+x*3])
                    {
                        if(current.rawData()[y*width*3+x*3+1] != data[y*width*3+x*3+1])
                        {
                            if(current.rawData()[y*width*3+x*3+2] != data[y*width*3+x*3+2])
                            {
                                point tmp;
                                tmp.x = x;
                                tmp.y = y;
                                differences.push_back(tmp);
                            }    
                        }   
                    }
                }
            }
            int xa = width, xb = 0, ya = height, yb = 0;
            for(int i = 0; i < differences.size(); i++)
            {
                if(differences[i].x < xa)
                {
                    xa = differences[i].x;
                }
                if(differences[i].x > xb)
                {
                    xb = differences[i].x;
                }
                if(differences[i].y < ya)
                {
                    ya = differences[i].y;
                }
                if(differences[i].y > yb)
                {
                    yb = differences[i].y;
                }
            }

            Frame next;

            if((xb-xa)*(yb-ya)*3 < differences.size()*3+differences.size()*4)
            {
                next.setType(FrameType::RECT);
                int w = (xb-xa);
                int h = (yb-ya);

                if(w == width && h == height)
                {
                    next.setType(FrameType::FULL);
                    frames.push_back(Frame(data, width, height));
                    current = Frame(data, width, height);
                    return;
                }

                unsigned char* rectData = alloc(w*h*3);
                
                for(int y = 0; y < h; y++)
                {   
                    for(x = 0; x < w; x++)
                    {
                        rectData[y*h*3+x*3] = data[(ya+y)*width*3+(xa+x)*3];
                        rectData[y*h*3+x*3+1] = data[(ya+y)*width*3+(xa+x)*3+1];
                        rectData[y*h*3+x*3+2] = data[(ya+y)*width*3+(xa+x)*3+2];
                    }
                }

                next.setX(xa);
                next.setY(ya);
                next.setData(rectData, w, h);
                free(rectData);
            }
            else
            {
                next.setType(FrameType::PIXELS);
                unsigned char* updateData = alloc(differences.size()*3);
                int u = 0;
                for(int i = 0; i < differences.size(); i++)
                {
                    updateData[u] = data[differences[i].y*width*3+differences[i].x*3];
                    updateData[u+1] = data[differences[i].y*width*3+differences[i].x*3+1];
                    updateData[u+2] = data[differences[i].y*width*3+differences[i].x*3+2];
                    u += 3;
                }
                free(updateData);
            }
            frames.push_back(next);
            current.updateByFrame(next);
        }
        
    }









};











