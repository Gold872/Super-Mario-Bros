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
   UNDERWATER,
   CASTLE,
   START_UNDERGROUND
};

struct LevelData {
   std::vector<std::unique_ptr<Map>> levelMaps;

   Vector2i playerStart;
   LevelType levelType;

   BackgroundColor levelBackgroundColor;

   int cameraMax;

   Vector2i nextLevel;

   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor,
                          bool, Vector2i>>
       warpPipeLocations;
   std::vector<std::tuple<Vector2i, MysteryBoxType>> questionBlockLocations;
   std::vector<std::tuple<Vector2i, MysteryBoxType>> mysteryBrickLocations;
   std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int>>
       movingPlatformDirections;
   std::vector<std::tuple<Vector2i, int, RotationDirection, int>> fireBarLocations;
};

class Level {
  public:
   Level() = default;

   ~Level() {
      maps.clear();
   }

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
       {"UNDERWATER", LevelType::UNDERWATER},
       {"CASTLE", LevelType::CASTLE},
       {"START_UNDERGROUND", LevelType::START_UNDERGROUND},
   };

   std::unordered_map<string, MysteryBoxType> mysteryBoxTypeString = {
       {"MUSHROOM", MysteryBoxType::MUSHROOM},
       {"COINS", MysteryBoxType::COINS},
       {"SUPER_STAR", MysteryBoxType::SUPER_STAR},
       {"RANDOM", MysteryBoxType::RANDOM},
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

         std::smatch pair_match;

         string s;
         while (getline(iss, s)) {
            std::regex_search(s, pair_match, pair_regex);

            for (unsigned int i = 1; i < pair_match.size(); i += 2) {
               pairArray.push_back(
                   Vector2i(std::stoi(pair_match[i]), std::stoi(pair_match[i + 1])));
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

   std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int>>
   loadMovingPlatform(string arrayPattern, string stringToSearch, string enumSearch) {
      std::regex arrayRegex(arrayPattern);
      std::regex enumPairRegex(enumSearch);

      std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int>>
          platformLocations;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;
         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, enumPairRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 7) {
                  Vector2i platformLocation(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  PlatformMotionType motionType = motionTypeString.at(pairMatch[i + 2]);
                  Direction movingDirection(directionString.at(pairMatch[i + 3]));
                  Vector2i platformMinMax(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));
                  int platformLength = std::stoi(pairMatch[i + 6]);

                  platformLocations.push_back(
                      std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int>(
                          platformLocation, motionType, movingDirection, platformMinMax,
                          platformLength));
               }
            }
         }
      }

      return platformLocations;
   }

   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor,
                          bool, Vector2i>>
   loadWarpPipeLocation(string regexPattern, string stringToSearch, string pipeSearch) {
      std::regex arrayRegex(regexPattern);
      std::regex pipeRegex(pipeSearch);

      std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool,
                             BackgroundColor, bool, Vector2i>>
          warpLocation;

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

                  bool underwater = pairMatch[i + 10] == "TRUE";

                  Vector2i newLevel =
                      Vector2i(std::stoi(pairMatch[i + 11]), std::stoi(pairMatch[i + 12]));

                  warpLocation.push_back(
                      std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool,
                                 BackgroundColor, bool, Vector2i>(
                          pipeCoordinates, teleportCoordinates, cameraCoordinates, inDirection,
                          outDirection, cameraFreeze, color, underwater, newLevel));
               }
            }
         }
      }
      return warpLocation;
   }

   std::vector<std::tuple<Vector2i, int, RotationDirection, int>> loadFireBar(string regexPattern,
                                                                              string stringToSearch,
                                                                              string searchBar) {
      std::regex arrayRegex(regexPattern);
      std::regex barRegex(searchBar);

      std::vector<std::tuple<Vector2i, int, RotationDirection, int>> barLocations;

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

                  barLocations.push_back(std::tuple<Vector2i, int, RotationDirection, int>(
                      barCoordinates, startAngle, rotationDirection, barLength));
               }
            }
         }
      }

      return barLocations;
   }

   Map* createMap() {
      Map* map(new Map());

      std::unique_ptr<Map> uniqueMap{map};

      data.levelMaps.emplace_back(std::move(uniqueMap));

      return map;
   }

   void loadLevelData(string levelProperties) {
      data.levelType = loadEnumData<LevelType>("LEVEL_TYPE" + DefaultRegexPattern, levelProperties,
                                               levelTypeString);

      data.levelBackgroundColor = loadEnumData<BackgroundColor>(
          "BACKGROUND_COLOR" + DefaultRegexPattern, levelProperties, backgroundColorString);

      data.playerStart =
          loadCoordinate("PLAYER_START(?:\\s)?=(?:\\s)?" + DefaultPairPattern, levelProperties);

      data.nextLevel =
          loadCoordinate("NEXT_LEVEL(?:\\s)?=(?:\\s)?" + DefaultPairPattern, levelProperties);

      data.cameraMax = loadIntData("CAMERA_MAX" + DefaultRegexPattern, levelProperties);

      data.warpPipeLocations =
          loadWarpPipeLocation("WARP_PIPE" + DefaultArrayPattern, levelProperties, warpPipePattern);

      data.questionBlockLocations = loadEnumCoordinateArray<MysteryBoxType>(
          "MYSTERY_BOX" + DefaultArrayPattern, levelProperties, CoordinateEnumPattern,
          mysteryBoxTypeString);

      data.mysteryBrickLocations = loadEnumCoordinateArray<MysteryBoxType>(
          "MYSTERY_BRICK" + DefaultArrayPattern, levelProperties, CoordinateEnumPattern,
          mysteryBoxTypeString);

      data.movingPlatformDirections = loadMovingPlatform("MOVING_PLATFORM" + DefaultArrayPattern,
                                                         levelProperties, platformPattern);

      data.fireBarLocations =
          loadFireBar("FIRE_BAR" + DefaultArrayPattern, levelProperties, fireBarPattern);
   }

   void clearLevelData() {
      data.levelType = LevelType::NONE;
      data.levelBackgroundColor = BackgroundColor::BLACK;
      data.playerStart = Vector2i(0, 0);
      data.nextLevel = Vector2i(0, 0);
      data.cameraMax = 0;
      data.warpPipeLocations.clear();
      data.questionBlockLocations.clear();
      data.mysteryBrickLocations.clear();
      data.movingPlatformDirections.clear();
      data.fireBarLocations.clear();
   }

   LevelData& getData() {
      return data;
   }

   std::vector<std::unique_ptr<Map>>& getMaps() {
      return data.levelMaps;
   }

   LevelType& getLevelType() {
      return data.levelType;
   }

  private:
   LevelData data;

   std::vector<std::unique_ptr<Map>> maps;

   std::vector<Vector2i> testLocations;

   string DefaultRegexPattern = /* SEARCH NAME */ "(?:\\s)?=(?:\\s)?(\\w+|[+-]*\\d+)";
   //   		":\\s(\\w+|[+-]*\\d+)";

   string DefaultArrayPattern =
       /* SEARCH NAME */ "(?:\\s)?=(?:\\s)?\\\\\\n([\\(\\)\\d\\s\\w,\\\\\\n]+)";

   //"SEARCH_NAME(?:\\s)?=(?:\\s)?(\\w+|[+-]*\\d+"
   //"TEST_COORDINATES(?:\\s)?=(?:\\s)?\\\\\\n([\\(\\)\\d\\s,]+)"

   // OG: \\s\\{\n([\n\\d\\s\\(\\),]+)\\}

   string DefaultPairPattern = "\\((\\d+), (\\d+)\\)";

   string CoordinateEnumPattern = "\\((\\d+), (\\d+)\\)\\s(\\w+)";

   string warpPipePattern =
       "\\((\\d+), (\\d+)\\)\\s\\((\\d+), (\\d+)\\)\\s\\((\\d+), "
       "(\\d+)\\)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s\\((\\d+), (\\d+)\\)";

   string platformPattern =
       "\\((\\d+), (\\d+)\\)\\s(\\w+)\\s(\\w+)\\s\\((\\d+), (\\d+)\\)\\s(\\d+)";

   string fireBarPattern = "\\((\\d+), (\\d+)\\)\\s(\\d+)\\s(\\w+)\\s(\\d+)";
};
