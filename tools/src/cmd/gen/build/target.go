// Copyright 2023 The Dawn & Tint Authors.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package build

import (
	"sort"

	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

// Directory holds information about a build target
type Target struct {
	// The target's name
	Name TargetName
	// The kind of target
	Kind TargetKind
	// The directory holding the target
	Directory *Directory
	// All project-relative paths of source files that are part of this target
	SourceFileSet container.Set[string]
	// Dependencies of this target
	Dependencies *Dependencies
	// An optional custom output name for the target
	OutputName string
	// An optional condition for building this target
	Condition Condition
}

// AddSourceFile adds the File to the target's source set
func (t *Target) AddSourceFile(f *File) {
	t.SourceFileSet.Add(f.Path())
	f.Target = t
}

// SourceFiles returns the sorted list of the target's source files
func (t *Target) SourceFiles() []*File {
	out := make([]*File, len(t.SourceFileSet))
	for i, name := range t.SourceFileSet.List() {
		out[i] = t.Directory.Project.Files[name]
	}
	return out
}

// SourceFiles returns the sorted list of the target's source files that have no build condition
func (t *Target) UnconditionalSourceFiles() []*File {
	return transform.Filter(t.SourceFiles(), func(t *File) bool { return t.Condition == nil })
}

// A collection of source files and dependencies sharing the same condition
type TargetConditional struct {
	Condition            Condition
	SourceFiles          []*File
	InternalDependencies []*Target
	ExternalDependencies []ExternalDependency
}

// A collection of source files and dependencies sharing the same condition
type TargetConditionals []*TargetConditional

// HasSourceFiles returns true if any of the conditionals in l have source files
func (l TargetConditionals) HasSourceFiles() bool {
	for _, c := range l {
		if len(c.SourceFiles) > 0 {
			return true
		}
	}
	return false
}

// HasDependencies returns true if any of the conditionals in l have internal or external dependencies
func (l TargetConditionals) HasDependencies() bool {
	for _, c := range l {
		if len(c.InternalDependencies) > 0 || len(c.ExternalDependencies) > 0 {
			return true
		}
	}
	return false
}

// Conditionals returns a sorted list of TargetConditional, which are grouped by condition
func (t *Target) Conditionals() TargetConditionals {
	m := container.NewMap[string, *TargetConditional]()
	for name := range t.SourceFileSet {
		file := t.Directory.Project.Files[name]
		if file.Condition != nil {
			c := m.GetOrCreate(file.Condition.String(), func() *TargetConditional {
				return &TargetConditional{Condition: file.Condition}
			})
			c.SourceFiles = append(c.SourceFiles, file)
		}
	}
	for name := range t.Dependencies.internal {
		dep := t.Directory.Project.Targets[name]
		if dep.Condition != nil {
			c := m.GetOrCreate(dep.Condition.String(), func() *TargetConditional {
				return &TargetConditional{Condition: dep.Condition}
			})
			c.InternalDependencies = append(c.InternalDependencies, dep)
		}
	}
	for name := range t.Dependencies.external {
		dep := t.Directory.Project.externals[name]
		if dep.Condition != nil {
			c := m.GetOrCreate(dep.Condition.String(), func() *TargetConditional {
				return &TargetConditional{Condition: dep.Condition}
			})
			c.ExternalDependencies = append(c.ExternalDependencies, dep)
		}
	}
	for _, c := range m {
		sort.Slice(c.SourceFiles, func(a, b int) bool { return c.SourceFiles[a].Name < c.SourceFiles[b].Name })
		sort.Slice(c.InternalDependencies, func(a, b int) bool { return c.InternalDependencies[a].Name < c.InternalDependencies[b].Name })
		sort.Slice(c.ExternalDependencies, func(a, b int) bool { return c.ExternalDependencies[a].Name < c.ExternalDependencies[b].Name })
	}
	return TargetConditionals(m.Values())
}
