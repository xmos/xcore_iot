@Library('xmos_jenkins_shared_library@v0.18.0') _

// wait here until specified artifacts appear
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
    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.1.4',
            description: 'The XTC tools version'
        )
    }    
    environment {
        DIST_PATH = "dist"
        CONDA_VENV = "xcore_sdk_venv"
    }        
    options {
        skipDefaultCheckout()
    }    
    stages {
        stage('Download artifacts') {
            steps {
                dir("${DIST_PATH}") {
                    downloadExtractZips(artifactUrls)
                    sh "ls -la"
                }
            }
        }
        stage('Install Dependencies') {
            steps {
                // withTools(params.TOOLS_VERSION) {
                //     installDependencies()
                // }
                sh "conda create --prefix ${CONDA_VENV} python=3.8"
                sh "conda activate ${CONDA_VENV}"
                sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                sh "conda deactivate"
            }
        }
        stage('Cleanup xtagctl') {
            steps {
                dir("${DIST_PATH}") {
                    withTools(params.TOOLS_VERSION) {
                        sh "conda activate ${CONDA_VENV}"
                        sh "xtagctl status"
                        sh "xtagctl reset_all XCORE-AI-EXPLORER"
                        sh "conda deactivate"
                        sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
                    }
                }
            }
        }
        stage('Run bare-metal examples') {
            steps {
                dir("${DIST_PATH}") {
                    withTools(params.TOOLS_VERSION) {
                        sh "conda activate ${CONDA_VENV}"
                        sh "xrun --xscope example_bare_metal_vww.xe 2>&1 | tee example_bare_metal_vww.log"
                        sh "conda deactivate"
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