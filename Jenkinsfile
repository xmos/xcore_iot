@Library('xmos_jenkins_shared_library@v0.18.0') _

def withXTAG(String target, Closure body) {
    def adapterID = sh "xtagctl acquire ${target}"
    body(adapterID)
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
        DIST_PATH = "dist"
        VENV_DIRNAME = ".venv"
        // VENV_PATH = "./jenkins_venv"   // NOTE: Needs to be prepended with ./
        // CONDA_PATH = "miniconda3"
        // CONDA_EXE = "${CONDA_PATH}/bin/conda"
        // CONDA_RUN = "${CONDA_EXE} run -p ${VENV_PATH}"
        // CONDA_RUN = ""
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
                dir("${DIST_PATH}") {
                    downloadExtractZips(artifactUrls)
                    // List extracted files for log
                    sh "ls -la"
                }
            }
        }
        stage('Create virtual environment') {
            steps {
                dir("${DIST_PATH}") {
                    sh "pyenv install -s 3.8.13"
                    sh "~/.pyenv/versions/3.8.13/bin/python -m venv ${VENV_DIRNAME}"
                    withVenv() {
                        sh "pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
                    }
                }
            }
        }
        // stage('Create virtual environment') {
        //     steps {
        //         dir("${DIST_PATH}") {
        //             // Install Conda
        //             sh "wget https://repo.anaconda.com/miniconda/Miniconda3-py38_4.11.0-Linux-x86_64.sh -O conda_install.sh"
        //             sh "bash conda_install.sh -b -p ${CONDA_PATH}"
        //             sh "rm -rf conda_install.sh"
        //         }
        //         dir("${DIST_PATH}") {
        //             // Create the Conda environment
        //             sh "${CONDA_EXE} create --prefix ${VENV_PATH} python=3.8"
        //         }
        //         dir("${DIST_PATH}") {
        //             // Install dependencies
        //             sh "${CONDA_RUN} pip install git+https://github0.xmos.com/xmos-int/xtagctl.git"
        //         }
        //     }
        // }
        stage('Cleanup xtagctl') {
            steps {
                dir("${DIST_PATH}") {
                    // Cleanup any xtagctl cruft from previous failed runs
                    withTools(params.TOOLS_VERSION) {
                        withVenv() {
                            sh "xtagctl reset_all XCORE-AI-EXPLORER"
                        }
                    }
                    sh "rm -f ~/.xtag/status.lock ~/.xtag/acquired"
                }
            }
        }
        stage('Run bare-metal examples') {
            steps {
                dir("${DIST_PATH}") {
                    withTools(params.TOOLS_VERSION) {
                        withVenv() {
                            withXTAG("xcore_sdk_test_rig") { adapterID ->
                                sh "../test/examples/run_bare_metal_vww_tests.sh ${adapterID}"
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