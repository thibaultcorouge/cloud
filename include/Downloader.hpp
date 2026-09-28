#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include <string>


class Downloader {
  public:
    Downloader();
    ~Downloader();

    bool download(const std::string& url, const std::string& outputPath);
};

#endif
