@Library('xmos_jenkins_shared_library@v0.18.0') _

def withXTAG(String target, Closure body) {
    // Acquire an xtag adapter-id by target name
    def adapterID = sh (script: "xtagctl acquire ${target}", returnStdout: true).trim()
    // Run the closure
    body(adapterID)
    // Release the xtag by adapter-id
    sh ("xtagctl release ${adapterID}")
}


// Wait here until specified artifacts appear
def artifactUrls = getGithubArtifactUrls([
    "xcore_sdk_bare-metal_example_apps"
    // "xcore_sdk_freertos_example_apps",
    // "xcore_sdk_freertos_usb_example_apps",
    // "xcore_sdk_host_apps",
    // "xcore_sdk_reference_apps",
    // "xcore_sdk_rtos_tests"
])

getApproval()

pipeline {
    agent {
        label 'sdk'
    }
    options {
        disableConcurrentBuilds()
        skipDefaultCheckout()
    }    
    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.1.4',
            description: 'The XTC tools version'
        )
    }    
    environment {
        PYTHON_VERSION = "3.8.11"
        VENV_DIRNAME = ".venv"
        DOWNLOAD_DIRNAME = "build"
        SDK_TEST_RIG_TARGET = "xcore_sdk_test_rig"
    }        
    stages {
        stage('Checkout') {
            steps {
                checkout scm
                sh "git clone git@github.com:xmos/xcore_sdk.git"
            }
        }        
        stage('Download artifacts') {
            steps {
                dir("$DOWNLOAD_DIRNAME") {
                    downloadExtractZips(artifactUrls)
                    // List extracted files for log
                    sh "ls -la"
                }
            }
        }
        stage('Create virtual environment') {
            steps {
                // Create venv
                sh "pyenv install -s $PYTHON_VERSION"
                sh "~/.pyenv/versions/$PYTHON_VERSION/bin/python -m venv $VENV_DIRNAME"
                // Install dependencies
                withVenv() {
                    // NOTE: only one dependency so not using a requirements.txt file here yet
                    sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                }
            }
        }
        stage('Cleanup xtagctl') {
            steps {
                // Cleanup any xtagctl cruft from previous failed runs
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        sh "xtagctl reset_all $SDK_TEST_RIG_TARGET"
                    }
                }
                sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
            }
        }
        stage('Run bare-metal examples') {
            steps {
                withTools(params.TOOLS_VERSION) {
                    withVenv {
                        script {
                            if (fileExists("$DOWNLOAD_DIRNAME/example_bare_metal_vww.xe")) {
                                withXTAG("$SDK_TEST_RIG_TARGET") { adapterID ->
                                    sh "test/examples/run_bare_metal_vww_tests.sh $adapterID"
                                }
                            } else {
                                echo 'SKIPPED: example_bare_metal_vww'
                            }
                        } 
                    }
                }
            }
        }
    }
    post {
        cleanup {
            cleanWs()
        }
    }
}