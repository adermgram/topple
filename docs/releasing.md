# Making a release

`.github/workflows/build.yml` builds the game on Windows, Linux and macOS for every push and keeps the packaged result as an artifact on the run. A release is made from a tag that starts with `v`; once all three builds succeed, the workflow attaches the packages to it.

**From a terminal.** Push the tag and the workflow creates the release itself, with notes generated from the commits:

```
git tag v1.0.0
git push origin v1.0.0
```

**On github.com.** Go to *Releases*, then *Draft a new release*. Under *Choose a tag* type a new tag such as `v1.0.0` and pick *Create new tag on publish*. Click *Generate release notes* and *Publish release*. The release shows up straight away and the packages are added a few minutes later, when the builds finish.

The *Actions* tab shows progress, and its *Run workflow* button builds without releasing anything. Locally, `cmake --install build --prefix <folder> --component game` produces the folder that goes into a package.

If a release goes wrong, delete it on GitHub, then remove the tag with `git push origin --delete v1.0.0` and `git tag -d v1.0.0`.
