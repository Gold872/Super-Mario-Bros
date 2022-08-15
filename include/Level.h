#pragma once

#include "ECS/Components.h"
#include "Map.h"
#include "Math.h"
#include "TextureManager.h"

#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using std::string;
using MovingPlatformData = std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, bool>;
using PlatformLevelData = std::tuple<Vector2i, Vector2i, int>;
using FireBarData = std::tuple<Vector2i, int, RotationDirection, int>;
using VineData =
    std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i, int, BackgroundColor, LevelType>;
using WarpPipeData = std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool,
                                BackgroundColor, LevelType, Vector2i>;

using FloatingText = std::tuple<Vector2i, string>;

enum class WarpPipeDirections
{
   NONE,
   UP,
   DOWN,
   LEFT,
   RIGHT
};

enum class LevelType
{
   NONE,
   OVERWORLD,
   UNDERGROUND,
   UNDERWATER,
   CASTLE,
   START_UNDERGROUND
};

struct LevelData {
   Vector2i playerStart;
   LevelType levelType;

   Vector2i cameraStart;

   BackgroundColor levelBackgroundColor;

   int cameraMax;

   Vector2i nextLevel;

   std::vector<WarpPipeData> warpPipeLocations;
   std::vector<MovingPlatformData> movingPlatformDirections;
   std::vector<PlatformLevelData> platformLevelLocations;
   std::vector<FireBarData> fireBarLocations;
   std::vector<VineData> vineLocations;
   std::vector<FloatingText> floatingTextLocations;
};

class Level {
  public:
   Level() = default;

   ~Level() {}

   std::unordered_map<string, Direction> directionString = {
       {"NONE", Direction::NONE}, {"UP", Direction::UP},       {"DOWN", Direction::DOWN},
       {"LEFT", Direction::LEFT}, {"RIGHT", Direction::RIGHT},
   };

   std::unordered_map<string, PlatformMotionType> motionTypeString = {
       {"NONE", PlatformMotionType::NONE},
       {"ONE_DIRECTION_REPEATED", PlatformMotionType::ONE_DIRECTION_REPEATED},
       {"ONE_DIRECTION_CONTINUOUS", PlatformMotionType::ONE_DIRECTION_CONTINUOUS},
       {"BACK_AND_FORTH", PlatformMotionType::BACK_AND_FORTH},
       {"GRAVITY", PlatformMotionType::GRAVITY},
   };

   std::unordered_map<string, RotationDirection> rotationString = {
       {"NONE", RotationDirection::NONE},
       {"CLOCKWISE", RotationDirection::CLOCKWISE},
       {"COUNTER_CLOCKWISE", RotationDirection::COUNTER_CLOCKWISE}};

   std::unordered_map<string, LevelType> levelTypeString = {
       {"NONE", LevelType::NONE},
       {"OVERWORLD", LevelType::OVERWORLD},
       {"UNDERGROUND", LevelType::UNDERGROUND},
       {"UNDERWATER", LevelType::UNDERWATER},
       {"CASTLE", LevelType::CASTLE},
       {"START_UNDERGROUND", LevelType::START_UNDERGROUND},
   };

   std::unordered_map<string, MysteryBoxType> mysteryBoxTypeString = {
       {"MUSHROOM", MysteryBoxType::MUSHROOM},
       {"COINS", MysteryBoxType::COINS},
       {"SUPER_STAR", MysteryBoxType::SUPER_STAR},
       {"ONE_UP", MysteryBoxType::ONE_UP},
   };

   std::unordered_map<string, BackgroundColor> backgroundColorString = {
       {"BLACK", BackgroundColor::BLACK},
       {"BLUE", BackgroundColor::BLUE},
   };

   Vector2i loadCoordinate(string regexPattern, string stringToSearch) {
      std::regex coordinates_regex(regexPattern);
      std::smatch matches;

      if (std::regex_search(stringToSearch, matches, coordinates_regex)) {
         return Vector2i(std::stoi(matches[1]), std::stoi(matches[2]));
      }

      return Vector2i(0, 0);
   }

   template <typename T>
   T loadEnumData(string regexPattern, string stringToSearch,
                  std::unordered_map<string, T> enumStringTable) {
      std::regex dataRegex(regexPattern);
      std::smatch matches;

      if (std::regex_search(stringToSearch, matches, dataRegex)) {
         return enumStringTable.at(matches[1]);
      }
      return T::NONE;
   }

   std::vector<Vector2i> loadCoordinateArray(string regexPattern, string stringToSearch,
                                             string pairs) {
      std::regex coordinates_regex(regexPattern);
      std::regex pair_regex(pairs);

      std::vector<Vector2i> pairArray;

      std::smatch coordinates_match;
      if (std::regex_search(stringToSearch, coordinates_match, coordinates_regex)) {
         std::istringstream iss(coordinates_match[1]);

         std::smatch pairMatch;

         string s;
         while (getline(iss, s)) {
            std::regex_search(s, pairMatch, pair_regex);

            for (unsigned int i = 1; i < pairMatch.size(); i += 2) {
               pairArray.push_back(Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1])));
            }
         }
      }
      return pairArray;
   }

   template <typename T>
   std::vector<std::tuple<Vector2i, T>> loadEnumCoordinateArray(
       string regexPattern, string stringToSearch, string enumSearch,
       std::unordered_map<string, T> enumStringTable) {
      std::regex arrayRegex(regexPattern);
      std::regex enumPairRegex(enumSearch);

      std::vector<std::tuple<Vector2i, T>> enumArray;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;
         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, enumPairRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 3) {
                  Vector2i pairCoordinates =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  T pairEnum = enumStringTable.at(pairMatch[i + 2]);

                  enumArray.push_back(std::tuple<Vector2i, T>(pairCoordinates, pairEnum));
               }
            }
         }
      }
      return enumArray;
   }

   int loadIntData(string regexPattern, string stringToSearch) {
      std::regex dataRegex(regexPattern);
      std::smatch matches;

      if (std::regex_search(stringToSearch, matches, dataRegex)) {
         return std::stoi(matches[1]);
      }
      return 0;
   }

   std::vector<FloatingText> loadFloatingText(string stringToSearch) {
      std::regex arrayRegex("FLOATING_TEXT" + arrayPattern);
      std::regex textRegex(floatingTextPattern);

      std::smatch arrayMatch;

      std::vector<FloatingText> floatingTextLocations;

      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, textRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 3) {
                  Vector2i textLocation =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));

                  string text = pairMatch[i + 2];

                  floatingTextLocations.push_back(FloatingText(textLocation, text));
               }
            }
         }
      }

      return floatingTextLocations;
   }

   std::vector<MovingPlatformData> loadMovingPlatforms(string stringToSearch) {
      std::regex arrayRegex("MOVING_PLATFORM" + arrayPattern);
      std::regex platformRegex(platformPattern);

      std::vector<MovingPlatformData> platformData;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;
         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, platformRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 7) {
                  Vector2i platformLocation(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  PlatformMotionType motionType = motionTypeString.at(pairMatch[i + 2]);
                  Direction movingDirection(directionString.at(pairMatch[i + 3]));
                  Vector2i platformMinMax(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));
                  bool rightShift = pairMatch[i + 6] == "TRUE";

                  platformData.push_back(MovingPlatformData(
                      platformLocation, motionType, movingDirection, platformMinMax, rightShift));
               }
            }
         }
      }

      return platformData;
   }

   std::vector<PlatformLevelData> loadPlatformLevels(string stringToSearch) {
      std::regex arrayRegex("PLATFORM_LEVEL" + arrayPattern);
      std::regex levelRegex(platformLevelPattern);

      std::vector<PlatformLevelData> platformData;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;
         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, levelRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 5) {
                  Vector2i leftPlatformLocation =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  Vector2i rightPlatformLocation =
                      Vector2i(std::stoi(pairMatch[i + 2]), std::stoi(pairMatch[i + 3]));
                  int pulleyLevel = std::stoi(pairMatch[i + 4]);

                  platformData.push_back(
                      PlatformLevelData(leftPlatformLocation, rightPlatformLocation, pulleyLevel));
               }
            }
         }
      }

      return platformData;
   }

   std::vector<WarpPipeData> loadWarpPipes(string stringToSearch) {
      std::regex arrayRegex("WARP_PIPE" + arrayPattern);
      std::regex pipeRegex(warpPipePattern);

      std::vector<WarpPipeData> pipeData;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;
         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, pipeRegex)) {
               for (unsigned i = 1; i < pairMatch.size(); i += 13) {
                  Vector2i pipeCoordinates =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  Vector2i teleportCoordinates =
                      Vector2i(std::stoi(pairMatch[i + 2]), std::stoi(pairMatch[i + 3]));
                  Vector2i cameraCoordinates =
                      Vector2i(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));

                  Direction inDirection = directionString.at(pairMatch[i + 6]);
                  Direction outDirection = directionString.at(pairMatch[i + 7]);

                  bool cameraFreeze = pairMatch[i + 8] == "TRUE";

                  BackgroundColor color = backgroundColorString.at(pairMatch[i + 9]);

                  LevelType levelType = levelTypeString.at(pairMatch[i + 10]);

                  Vector2i newLevel =
                      Vector2i(std::stoi(pairMatch[i + 11]), std::stoi(pairMatch[i + 12]));

                  pipeData.push_back(WarpPipeData(pipeCoordinates, teleportCoordinates,
                                                  cameraCoordinates, inDirection, outDirection,
                                                  cameraFreeze, color, levelType, newLevel));
               }
            }
         }
      }
      return pipeData;
   }

   std::vector<FireBarData> loadFireBars(string stringToSearch) {
      std::regex arrayRegex("FIRE_BAR" + arrayPattern);
      std::regex barRegex(fireBarPattern);

      std::vector<FireBarData> barData;

      std::smatch arrayMatch;

      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, barRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 5) {
                  Vector2i barCoordinates =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  int startAngle = std::stoi(pairMatch[i + 2]);
                  RotationDirection rotationDirection = rotationString.at(pairMatch[i + 3]);
                  int barLength = std::stoi(pairMatch[i + 4]);

                  barData.push_back(
                      FireBarData(barCoordinates, startAngle, rotationDirection, barLength));
               }
            }
         }
      }

      return barData;
   }

   std::vector<VineData> loadVines(string stringToSearch) {
      std::regex arrayRegex("VINE" + arrayPattern);
      std::regex vineRegex(vinePattern);

      std::vector<VineData> vineLocations;

      std::smatch arrayMatch;

      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, vineRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 12) {
                  Vector2i blockLocation =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  Vector2i teleportLocation =
                      Vector2i(std::stoi(pairMatch[i + 2]), std::stoi(pairMatch[i + 3]));
                  Vector2i cameraCoordinates =
                      Vector2i(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));
                  int resetYValue = std::stoi(pairMatch[i + 6]);
                  Vector2i resetTeleportLocation =
                      Vector2i(std::stoi(pairMatch[i + 7]), std::stoi(pairMatch[i + 8]));
                  int newCameraMax = std::stoi(pairMatch[i + 9]);
                  BackgroundColor backgroundColor = backgroundColorString.at(pairMatch[i + 10]);
                  LevelType levelType = levelTypeString.at(pairMatch[i + 11]);

                  vineLocations.push_back(
                      VineData(blockLocation, teleportLocation, cameraCoordinates, resetYValue,
                               resetTeleportLocation, newCameraMax, backgroundColor, levelType));
               }
            }
         }
      }

      return vineLocations;
   }

   void loadLevelData(string levelProperties) {
      data.levelType = loadEnumData<LevelType>("LEVEL_TYPE" + normalRegexPattern, levelProperties,
                                               levelTypeString);

      data.levelBackgroundColor = loadEnumData<BackgroundColor>(
          "BACKGROUND_COLOR" + normalRegexPattern, levelProperties, backgroundColorString);

      data.playerStart =
          loadCoordinate("PLAYER_START(?:\\s)?=(?:\\s)?" + vectorPattern, levelProperties);

      data.nextLevel =
          loadCoordinate("NEXT_LEVEL(?:\\s)?=(?:\\s)?" + vectorPattern, levelProperties);

      data.cameraStart =
          loadCoordinate("CAMERA_START(?:\\s)?=(?:\\s)?" + vectorPattern, levelProperties);

      data.cameraMax = loadIntData("CAMERA_MAX" + normalRegexPattern, levelProperties);

      data.floatingTextLocations = loadFloatingText(levelProperties);

      data.warpPipeLocations = loadWarpPipes(levelProperties);

      data.movingPlatformDirections = loadMovingPlatforms(levelProperties);

      data.platformLevelLocations = loadPlatformLevels(levelProperties);

      data.fireBarLocations = loadFireBars(levelProperties);

      data.vineLocations = loadVines(levelProperties);
   }

   void clearLevelData() {
      data.levelType = LevelType::NONE;
      data.levelBackgroundColor = BackgroundColor::BLACK;
      data.playerStart = Vector2i(0, 0);
      data.nextLevel = Vector2i(0, 0);
      data.cameraMax = 0;
      data.floatingTextLocations.clear();
      data.warpPipeLocations.clear();
      data.movingPlatformDirections.clear();
      data.platformLevelLocations.clear();
      data.fireBarLocations.clear();
      data.vineLocations.clear();
   }

   LevelData& getData() {
      return data;
   }

  private:
   LevelData data;

   string normalRegexPattern = "(?:\\s)?=(?:\\s)?(\\w+|[+-]*\\d+)";

   string arrayPattern = "(?:\\s)?=(?:\\s)?\\\\\\n([\\(\\)\\d\\s\\w,\\\\\\n]+)";

   string enumPattern = "(\\w+)";

   string vectorPattern = "\\((\\d+), (\\d+)\\)";

   string intPattern = "(\\d+)";

   string stringPattern = "\\((.*)\\)";

   string warpPipePattern = vectorPattern + " " + vectorPattern + " " + vectorPattern + " " +
                            enumPattern + " " + enumPattern + " " + enumPattern + " " +
                            enumPattern + " " + enumPattern + " " + vectorPattern;

   string platformPattern = vectorPattern + " " + enumPattern + " " + enumPattern + " " +
                            vectorPattern + " " + enumPattern;

   string platformLevelPattern = vectorPattern + " " + vectorPattern + " " + intPattern;

   string fireBarPattern = vectorPattern + " " + intPattern + " " + enumPattern + " " + intPattern;

   string vinePattern = vectorPattern + " " + vectorPattern + " " + vectorPattern + " " +
                        intPattern + " " + vectorPattern + " " + intPattern + " " + enumPattern +
                        " " + enumPattern;

   string floatingTextPattern = vectorPattern + " " + stringPattern;
};
