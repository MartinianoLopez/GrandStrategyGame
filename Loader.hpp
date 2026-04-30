#pragma once
#include <string>
#include "GameData.hpp"

// loadProvincesImage  Province.bmp ( Image where each province has an especific distintive color )
// loadDefinitions     ProvinceColor ---> provinceId                                                                 Definition.csv     1(id) ;128;34;64(color)
// loadProvincesFiles                     provinceId ---> CountryTag                                                 provinces Folder   1-UppLand.txt ---> owner = SWE
// loadCountryTagToCountryName                            CountryTag ---> CountryName                                00_countries.txt   SWE = "countries/Sweden.txt"
// loadOwnerToColor                                                       CountryName ---> Country Color             Countries Folder   Sweden.txt ---> color = { 157  51  167 }
// findFrontiers       <ProvinceColor1, ProvinceColor2>, Vector<FrontierPoints> (represents the frontier between two provinces)
// initCountries       image with the countries painted on

void loadAssets(GameData& state, SDL_Renderer* renderer);

SDL_Surface* 
loadProvincesImage(const std::string& filepath);   

std::map<uint32_t, uint32_t> 
loadDefinitions(const std::string& filepath);  

std::map<uint32_t, std::string> 
loadProvincesFiles(const std::string& dirpath); 

std::map<std::string, std::string>
loadCountryNames(const std::string& filepath);

std::map<std::string, uint32_t> 
loadOwnerToColor(const std::string& filepath);              

std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_Point>>
findFrontiers(SDL_Surface* img);             

SDL_Surface* 
createFrontiersSurface(GameData& state);  

SDL_Surface*
initCountries(const GameData& state);

std::map<uint32_t, SDL_Point> initProvincesCenters(const GameData& state);