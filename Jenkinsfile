@Library('xmos_jenkins_shared_library@develop') _

// wait here until specified artifacts appear
def docArtifactUrls = getGithubArtifactUrls([
    "xcore_sdk_bare-metal_example_apps"
    // "xcore_sdk_freertos_example_apps",
    // "xcore_sdk_freertos_usb_example_apps",
    // "xcore_sdk_host_apps",
    // "xcore_sdk_reference_apps",
    // "xcore_sdk_rtos_tests"
])

def distDir = "dist"

getApproval()

pipeline {
    agent {
        label 'linux'
    }
    stages {
        stage('download artifacts') {
            steps {
                dir(${distDir}) {
                    downloadExtractZips(artifactUrls)
                    sh "tar -xf xcore_sdk_bare-metal_example_apps"
                }
            }
        }
        stage('run bare-metal examples') {
            steps {
                dir(${distDir}) {
                    sh "ls -la"
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