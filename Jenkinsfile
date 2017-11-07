pipeline {
    agent {
        label 'cmake'
    }
    stages {
        stage('Build and Test') {
            parallel {
                stage('Linux/GCC') {
                    agent {
                        label 'linux && gcc'
                    }
                    steps {
                        sh '''
                            rm -rf build/
                            mkdir build
                            cd build
                            gcc --version
                            cmake -G "Unix Makefiles" ../
                            make
                        '''
                    }
                }
                stage('Linux/Clang') {
                    agent {
                        label 'linux && clang'
                    }
                    steps {
                        sh '''
                            export CC=clang
                            export CXX=clang++
                            rm -rf build/
                            mkdir build
                            cd build
                            clang --version
                            cmake -G "Unix Makefiles" ../
                            make
                        '''
                    }
                }
                stage('Windows/GCC') {
                    agent {
                        label 'windows && gcc'
                    }
                    steps {
                        bat'''
                            set PATH=%PATH%;%MINGW64%;
                            rmdir /Q /S build
                            mkdir build
                            cd build
                            gcc --version
                            call cmake -G "MinGW Makefiles" ../
                            mingw32-make
                        '''
                    }
                }
                stage('Windows/Clang') {
                    agent {
                        label 'windows && clang'
                    }
                    steps {
                        bat'''
                            set PATH=%PATH%;%MINGW64%;
                            set CC=clang
                            set CXX=clang++
                            rmdir /Q /S build
                            mkdir build
                            cd build
                            clang --version
                            call cmake -G "MinGW Makefiles" ../
                            mingw32-make
                        '''
                    }
                }
            }
        }
    }
}