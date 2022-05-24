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

   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor,
                          LevelType, Vector2i>>
       warpPipeLocations;
   std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int, bool>>
       movingPlatformDirections;
   std::vector<std::tuple<Vector2i, int, RotationDirection, int>> fireBarLocations;
   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i>> vineLocations;
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

   std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int, bool>>
   loadMovingPlatform(string arrayPattern, string stringToSearch, string enumSearch) {
      std::regex arrayRegex(arrayPattern);
      std::regex enumPairRegex(enumSearch);

      std::vector<std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int, bool>>
          platformLocations;

      std::smatch arrayMatch;
      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;
         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, enumPairRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 8) {
                  Vector2i platformLocation(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  PlatformMotionType motionType = motionTypeString.at(pairMatch[i + 2]);
                  Direction movingDirection(directionString.at(pairMatch[i + 3]));
                  Vector2i platformMinMax(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));
                  int platformLength = std::stoi(pairMatch[i + 6]);
                  bool rightShift = pairMatch[i + 7] == "TRUE";

                  platformLocations.push_back(
                      std::tuple<Vector2i, PlatformMotionType, Direction, Vector2i, int, bool>(
                          platformLocation, motionType, movingDirection, platformMinMax,
                          platformLength, rightShift));
               }
            }
         }
      }

      return platformLocations;
   }

   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool, BackgroundColor,
                          LevelType, Vector2i>>
   loadWarpPipeLocation(string regexPattern, string stringToSearch, string pipeSearch) {
      std::regex arrayRegex(regexPattern);
      std::regex pipeRegex(pipeSearch);

      std::vector<std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool,
                             BackgroundColor, LevelType, Vector2i>>
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

                  LevelType levelType = levelTypeString.at(pairMatch[i + 10]);

                  Vector2i newLevel =
                      Vector2i(std::stoi(pairMatch[i + 11]), std::stoi(pairMatch[i + 12]));

                  warpLocation.push_back(
                      std::tuple<Vector2i, Vector2i, Vector2i, Direction, Direction, bool,
                                 BackgroundColor, LevelType, Vector2i>(
                          pipeCoordinates, teleportCoordinates, cameraCoordinates, inDirection,
                          outDirection, cameraFreeze, color, levelType, newLevel));
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

   std::vector<std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i>> loadVines(
       string regexPattern, string stringToSearch, string searchVines) {
      std::regex arrayRegex(regexPattern);
      std::regex vineRegex(searchVines);

      std::vector<std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i>> vineLocations;

      std::smatch arrayMatch;

      if (std::regex_search(stringToSearch, arrayMatch, arrayRegex)) {
         std::istringstream iss(arrayMatch[1]);

         std::smatch pairMatch;

         string s;

         while (getline(iss, s)) {
            if (std::regex_search(s, pairMatch, vineRegex)) {
               for (unsigned int i = 1; i < pairMatch.size(); i += 9) {
                  Vector2i blockLocation =
                      Vector2i(std::stoi(pairMatch[i]), std::stoi(pairMatch[i + 1]));
                  Vector2i teleportLocation =
                      Vector2i(std::stoi(pairMatch[i + 2]), std::stoi(pairMatch[i + 3]));
                  Vector2i cameraCoordinates =
                      Vector2i(std::stoi(pairMatch[i + 4]), std::stoi(pairMatch[i + 5]));
                  int resetYValue = std::stoi(pairMatch[i + 6]);
                  Vector2i resetTeleportLocation =
                      Vector2i(std::stoi(pairMatch[i + 7]), std::stoi(pairMatch[i + 8]));

                  vineLocations.push_back(std::tuple<Vector2i, Vector2i, Vector2i, int, Vector2i>(
                      blockLocation, teleportLocation, cameraCoordinates, resetYValue,
                      resetTeleportLocation));
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
          loadCoordinate("PLAYER_START(?:\\s)?=(?:\\s)?" + pairPattern, levelProperties);

      data.nextLevel = loadCoordinate("NEXT_LEVEL(?:\\s)?=(?:\\s)?" + pairPattern, levelProperties);

      data.cameraStart =
          loadCoordinate("CAMERA_START(?:\\s)?=(?:\\s)?" + pairPattern, levelProperties);

      data.cameraMax = loadIntData("CAMERA_MAX" + normalRegexPattern, levelProperties);

      data.warpPipeLocations =
          loadWarpPipeLocation("WARP_PIPE" + arrayPattern, levelProperties, warpPipePattern);

      data.movingPlatformDirections =
          loadMovingPlatform("MOVING_PLATFORM" + arrayPattern, levelProperties, platformPattern);

      data.fireBarLocations =
          loadFireBar("FIRE_BAR" + arrayPattern, levelProperties, fireBarPattern);

      data.vineLocations = loadVines("VINE" + arrayPattern, levelProperties, vinePattern);
   }

   void clearLevelData() {
      data.levelType = LevelType::NONE;
      data.levelBackgroundColor = BackgroundColor::BLACK;
      data.playerStart = Vector2i(0, 0);
      data.nextLevel = Vector2i(0, 0);
      data.cameraMax = 0;
      data.warpPipeLocations.clear();
      data.movingPlatformDirections.clear();
      data.fireBarLocations.clear();
      data.vineLocations.clear();
   }

   LevelData& getData() {
      return data;
   }

  private:
   LevelData data;

   string normalRegexPattern = /* SEARCH NAME */ "(?:\\s)?=(?:\\s)?(\\w+|[+-]*\\d+)";

   string arrayPattern =
       /* SEARCH NAME */ "(?:\\s)?=(?:\\s)?\\\\\\n([\\(\\)\\d\\s\\w,\\\\\\n]+)";

   string pairPattern = "\\((\\d+), (\\d+)\\)";

   string coordinateEnumPattern = "\\((\\d+), (\\d+)\\)\\s(\\w+)";

   string warpPipePattern =
       "\\((\\d+), (\\d+)\\)\\s\\((\\d+), (\\d+)\\)\\s\\((\\d+), "
       "(\\d+)\\)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s(\\w+)\\s\\((\\d+), (\\d+)\\)";

   string platformPattern =
       "\\((\\d+), (\\d+)\\)\\s(\\w+)\\s(\\w+)\\s\\((\\d+), (\\d+)\\)\\s(\\d+)\\s(\\w+)";

   string fireBarPattern = "\\((\\d+), (\\d+)\\)\\s(\\d+)\\s(\\w+)\\s(\\d+)";

   string vinePattern =
       "\\((\\d+), (\\d+)\\)\\s\\((\\d+), (\\d+)\\)\\s\\((\\d+), (\\d+)\\)\\s(\\d+)\\s\\((\\d+), "
       "(\\d+)\\)";
};
