#include <cstdint>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using std::numeric_limits;
using std::unordered_map;
using std::string;
using std::string_view;
using std::stringstream;
using std::vector;
using std::cout;
using std::cerr;
using std::min;
using std::max;
using std::fstream;
using std::ios;

struct rgb_color {
    public:
    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        uint32_t hex;
        uint8_t rgba[4];
        
    };

    public:
    rgb_color() : hex(0) {

    }
    rgb_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = numeric_limits<uint8_t>::max()) : r(r), g(g), b(b), a(a) {
        
    }
    rgb_color(uint32_t hex) : r((hex >> 24) & 0xFF), g((hex >> 16) & 0xFF), b((hex >> 8) & 0xFF), a(hex & 0xFF) {

    }

};
struct image_data {
    int width;
    int height;
    rgb_color* data;
    
};
struct color_option_mask {
    public:
    rgb_color color;
    bool mask[4];

    public:
    color_option_mask() : color(), mask{true, true, true, true} {
        
    }
    color_option_mask(rgb_color color, bool mask1 = true, bool mask2 = true, bool mask3 = true, bool mask4 = true) : color(color), mask{mask1, mask2, mask3, mask4} {

    }
};

template<class StringType, class PredT_>
vector<string_view> split(const StringType& str, PredT_ pred) {
    if (str.empty())
        return vector<string_view>(); 

    vector<string_view> output;

    string_view::size_type prevPos = 0;

    for (string_view::size_type i = 0; i < str.size(); ++i) {
        if (pred(str[i])) {
            if ((prevPos == i + 1) || (prevPos == i))
                continue;
            output.emplace_back(&str[prevPos], i - prevPos);
            prevPos = i + 1;
        }
    }
    if (prevPos != str.size() - 1)
        output.emplace_back(&str[prevPos], str.size() - prevPos); // Last word

    return output;
}
unordered_map<string, color_option_mask> get_name_color_pairs() {
    return unordered_map<string, color_option_mask>(
        {
            {"white",           color_option_mask(rgb_color(0xFFFFFFFF),    1,1,1,0)},
            {"gray",            color_option_mask(rgb_color(0x7F7F7FFF),    1,1,1,0)},
            {"lead",            color_option_mask(rgb_color(0x3E3E4DFF),    1,1,1,0)},
            {"black",           color_option_mask(rgb_color(0x000000FF),    1,1,1,0)},
            {"opaque",          color_option_mask(rgb_color(0x000000FF),    0,0,0,1)},
            {"ghostly",         color_option_mask(rgb_color(0x0000007F),    0,0,0,1)},
            {"transparent",     color_option_mask(rgb_color(0x00000000),    0,0,0,1)},
            {"red",             color_option_mask(rgb_color(0xFF0000FF),    1,1,1,0)},
            {"cherry",          color_option_mask(rgb_color(0x990F02FF),    1,1,1,0)},
            {"pink",            color_option_mask(rgb_color(0xFFC0CBFF),    1,1,1,0)},
            {"crimson",         color_option_mask(rgb_color(0x8c0000FF),    1,1,1,0)},
            {"green",           color_option_mask(rgb_color(0x00FF00FF),    1,1,1,0)},
            {"forest",          color_option_mask(rgb_color(0x172808FF),    1,1,1,0)},
            {"olive",           color_option_mask(rgb_color(0x3C4C24FF),    1,1,1,0)},
            {"lime",            color_option_mask(rgb_color(0x32DC32FF),    1,1,1,0)},
            {"blue",            color_option_mask(rgb_color(0x0000FFFF),    1,1,1,0)},
            {"sky",             color_option_mask(rgb_color(0x82C8E5FF),    1,1,1,0)},
            {"ice",             color_option_mask(rgb_color(0x00A1CEFF),    1,1,1,0)},
            {"ocean",           color_option_mask(rgb_color(0x1A224CFF),    1,1,1,0)},
            {"indigo",          color_option_mask(rgb_color(0x281E5DFF),    1,1,1,0)},
            {"yellow",          color_option_mask(rgb_color(0xFFF200FF),    1,1,1,0)},
            {"amber",           color_option_mask(rgb_color(0xFFBF00FF),    1,1,1,0)},
            {"brown",           color_option_mask(rgb_color(0x895129FF),    1,1,1,0)},
        }
    );
}

rgb_color avange_color(const vector<color_option_mask>& colors) {
    float rf, gf, bf, af;
    rf = gf = bf = af = 0.0f;

    int avangeMatterCount[4] {};

    for (auto powerColor : colors) {
        if (powerColor.mask[0]) {
            rf += (float)powerColor.color.r;
            ++avangeMatterCount[0];
        }
        if (powerColor.mask[1]) {
            gf += (float)powerColor.color.g;
            ++avangeMatterCount[1];
        }
        if (powerColor.mask[2]) {
            bf += (float)powerColor.color.b;
            ++avangeMatterCount[2];
        }
        if (powerColor.mask[3]) {
            af += (float)powerColor.color.a;
            ++avangeMatterCount[3];
        }

    }
    return rgb_color(
        uint8_t(rf / (float)avangeMatterCount[0]),
        uint8_t(gf / (float)avangeMatterCount[1]),
        uint8_t(bf / (float)avangeMatterCount[2]),
        avangeMatterCount[3] == 0 ? 255 : uint8_t(af / (float)avangeMatterCount[3]));

}
// 'decoder' 
string find_closest_color_name(rgb_color target_color, const unordered_map<string, color_option_mask>& nameColorPairs) {
    double minDistance1 = std::numeric_limits<double>::max();
    double minDistance2 = std::numeric_limits<double>::max();
    string closestColorName1;
    string closestColorName2;

    for (const auto& pair : nameColorPairs) {
        const rgb_color& color = pair.second.color;
        double distance = sqrt(
            abs((float)color.r - (float)target_color.r) +
            abs((float)color.g - (float)target_color.g) +
            abs((float)color.b - (float)target_color.b) +
            abs((float)color.a - (float)target_color.a)
        );

        if (distance < minDistance1) {
            minDistance1 = distance;
            closestColorName1 = pair.first;
        } else if (distance < minDistance2) {
            minDistance2 = distance;
            closestColorName2 = pair.first;
        }
    }
    

    return minDistance2 < (minDistance1 * 1.5f) ? (closestColorName1 + " and " + closestColorName2) : closestColorName1;
}
// coolnameimageformat header:

// reallycoolnameimageformat!                   | 'magic number'
// image with extent x y                        | image properties. x and y - variables
// and metadata with length len metadata        | metadata. len and metadata - variables. Space between len and metadata! 
// . . .                                        | colors data
template<class StreamT>
void stream_code_into_reallycoolnameimageformat(StreamT& stream, const image_data& image, const unordered_map<string, color_option_mask>& nameColorPairs) {
    const rgb_color* rawData = image.data;
    stream    << "reallycoolnameimageformat!\nimage with extent " << image.width << " " << image.height << "\n"
                    << "and metadata with length 0\n";
    for (int i = 0; i < (image.width * image.height); ++i) {
        stream << "a " << find_closest_color_name(rawData[i], nameColorPairs) << " color\n";
    }
}
string code_into_reallycoolnameimageformat(const image_data& image, const unordered_map<string, color_option_mask>& nameColorPairs) {
    const rgb_color* rawData = image.data;
    stringstream resultStream;
    stream_code_into_reallycoolnameimageformat(resultStream, image, nameColorPairs);
    return resultStream.str();
}

image_data decode_from_reallycoolnameimageformat(const string& data, const unordered_map<string, color_option_mask>& nameColorPairs) {
    image_data result;
    size_t currentPixelIndex = 0;

    bool aBeen = false;
    bool colorDecorator = false;
    const auto splited = split(data, [](char x){ return std::isspace(x);});
    if (splited.size() < 11) {
        cerr << "Too few words in file to be reallycoolnameimageformat.";
        exit(EXIT_FAILURE);
    }
    if (splited[0].compare("reallycoolnameimageformat!") != 0) {
        cerr << "This file isnt reallycoolnameimageformat.";
        exit(EXIT_FAILURE);
    }
    if ((splited[1].compare("image") != 0) ||
        (splited[2].compare("with") != 0) ||
        (splited[3].compare("extent") != 0)) {

        cerr << "Invalid image extent properties.";
        exit(EXIT_FAILURE);
    }
    result.width = std::stoi(string(splited[4]));
    result.height = std::stoi(string(splited[5]));
    if (result.width == 0 ||
        result.height == 0) {
        
        cerr << "Invalid image extent properties. Width: " << result.width << " height: " << result.height << ".";
        exit(EXIT_FAILURE);
    }
    if ((splited[6].compare("and") != 0) ||
        (splited[7].compare("metadata") != 0) ||
        (splited[8].compare("with") != 0) ||
        (splited[9].compare("length") != 0) ||
        (splited[10].compare("0") != 0)) {

        cerr << "Invalid image metadata properties.";
        exit(EXIT_FAILURE);
    }

    result.data = new rgb_color[result.width * result.height];
    rgb_color* rawData = result.data;

    vector<vector<color_option_mask>> colors;
    colors.resize(1);
    for (auto iter = splited.begin() + 10; iter != splited.end(); ++iter) {
        const auto& i = *iter;
        if (colorDecorator) {
            colors.back().emplace_back(nameColorPairs.at(string(i)));
            colorDecorator = false;

        } else if (i.compare("color") == 0) {
            rgb_color currentColor;
            for (const auto& powerColors : colors) {
                const rgb_color avange = avange_color(powerColors);
                currentColor.r = min(255, (uint16_t)currentColor.r + (uint16_t)avange.r);
                currentColor.b = min(255, (uint16_t)currentColor.b + (uint16_t)avange.b);
                currentColor.g = min(255, (uint16_t)currentColor.g + (uint16_t)avange.g);
                currentColor.a = min(255, (uint16_t)currentColor.a + (uint16_t)avange.a);
            }
            rawData[currentPixelIndex] = currentColor;
            ++currentPixelIndex;
            colors.clear();
            colors.resize(1);

        } else if ((i.compare("a") == 0) || (aBeen && (i.compare("and") == 0))) {
            aBeen = true;
            colorDecorator = true;

        } else if (i.compare("with") == 0) {
            colors.emplace_back();
            colorDecorator = true;
        }

    }
    return result;
}
string get_file_extension(const string& filePath) {
    size_t lastDotPosition = filePath.size() + 1;
    for (size_t i = 0; i < filePath.size(); ++i) {
        if (filePath[i] == '.')
            lastDotPosition = i;
    }
    if (lastDotPosition == (filePath.size() + 1))
        return string();
    else 
        return filePath.substr(lastDotPosition + 1, filePath.size() - lastDotPosition - 1);
}
const char* get_usage_message() {
    return 
        "-h/--help - show this message\n"
        "-i [FILE PATH] - set input image file\n"
        "-o [FILE PATH] - set output image file\n"
        "-m [c/d] - set de-/code mode.\n"
        "\tIf mode is 'c', the output file format will be reallycoolnameimageformat, the input will be any image file. 'c' mode by default.\n"
        "\tIf mode is 'd', the output file format will use the file extension specified in the file path or 'jpg' by default.\n";
}
int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Too few arguments. See -h/--help .";
        exit(EXIT_FAILURE);
    }

    const vector<string> arguments(argv + 1, argv + argc);

    try {
        if ((arguments.at(0) == "-h") || (arguments.at(0) == "--help")) {
            cout << get_usage_message();
            exit(EXIT_SUCCESS);
        }

        if ((arguments.size() % 2 == 1) || (arguments.size() > 6)) {
            cerr << "Invalid argument count. See -h/--help .";
            exit(EXIT_FAILURE);
        }

        string inputFile;
        string outputFile;
        bool isDecodeMode = true;

        for (size_t i = 0; i < arguments.size(); i += 2) {
            if (arguments[i] == "-i") {
                inputFile = arguments[i + 1];
            } else if (arguments[i] == "-o") {
                outputFile = arguments[i + 1];
            } else if (arguments[i] == "-m") {
                if (arguments[i + 1] == "c") {
                    isDecodeMode = false;
                } else if (arguments[i + 1] == "d") {
                    isDecodeMode = true;
                } else {
                    cerr << arguments[i + 1] << "is invalid mode. See -h/--help .";
                    exit(EXIT_FAILURE);
                }
            } else {
                cerr << arguments[i + 1] << "is invalid argument. See -h/--help .";
                exit(EXIT_FAILURE);
            }
        }

        const unordered_map<string, color_option_mask> nameColorPairs = get_name_color_pairs();
        if (isDecodeMode) {
            int x, y, c;
            string reallycoolnameimageformatData;
            {
                fstream reallycoolnameimageformatFile(inputFile, ios::in);
                if (!reallycoolnameimageformatFile.is_open()) {
                    cerr << "Failed to open file named " << inputFile << " .";
                    exit(EXIT_FAILURE);
                }
                stringstream stream;
                stream << reallycoolnameimageformatFile.rdbuf();
                reallycoolnameimageformatData = stream.str();
            }

            const image_data image = decode_from_reallycoolnameimageformat(reallycoolnameimageformatData, nameColorPairs);

            const string fileExtension = get_file_extension(outputFile);
            if (fileExtension == "png") {
                stbi_write_png(outputFile.c_str(), image.width, image.height, 4, image.data, image.width * 4);
            } else if (fileExtension == "bpm") {
                stbi_write_bmp(outputFile.c_str(), image.width, image.height, 4, image.data);
            } else if (fileExtension == "jpg") {
                stbi_write_jpg(outputFile.c_str(), image.width, image.height, 4, image.data, 100);
            } else {
                stbi_write_jpg((outputFile + ".jpg").c_str(), image.width, image.height, 4, image.data, 100);
            }

            stbi_image_free(image.data);
        } else {
            image_data image;
            {
                int c;
                image.data = reinterpret_cast<rgb_color*>(stbi_load(inputFile.c_str(), &image.width, &image.height, &c, 4));
                if (image.data == nullptr) {
                    cerr << "Failed to load image data from " << inputFile << " .";
                    exit(EXIT_FAILURE);
                }   
            }
            
            fstream reallycoolnameimageformatFile(
                get_file_extension(outputFile) == "reallycoolnameimageformat" ? outputFile.c_str() : (outputFile + ".reallycoolnameimageformat").c_str(),
                ios::out);

            stream_code_into_reallycoolnameimageformat(reallycoolnameimageformatFile, image, nameColorPairs);
            
            stbi_image_free(image.data);
        }
    } catch (const std::exception& e) {
        cerr << e.what();
    }

}