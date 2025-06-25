# Resizable Panels Implementation in ImGui

This document provides a comprehensive guide on implementing resizable panels in ImGui applications, covering the concepts, implementation details, and best practices used in the Wave Simulator Pro project.

## Table of Contents

1. [Overview](#overview)
2. [Core Concepts](#core-concepts)
3. [Implementation Architecture](#implementation-architecture)
4. [Step-by-Step Implementation](#step-by-step-implementation)
5. [Advanced Features](#advanced-features)
6. [Best Practices](#best-practices)
7. [Troubleshooting](#troubleshooting)
8. [Performance Considerations](#performance-considerations)

## Overview

Resizable panels provide users with the ability to customize their workspace by adjusting panel sizes according to their needs. This feature enhances user experience by allowing flexible layouts while maintaining professional appearance and functionality.

### Key Benefits
- **User Customization**: Users can adapt the interface to their workflow
- **Better Space Utilization**: Maximize important content areas
- **Professional Feel**: Modern applications expect resizable interfaces
- **Accessibility**: Users can adjust sizes for better readability

## Core Concepts

### 1. Panel Structure
```
┌─────────────────────────────────────────────────────────┐
│                    Menu Bar                             │
├─────────────┬─────────────────────────┬─────────────────┤
│             │                         │                 │
│   Left      │S                        │S     Right      │
│   Panel     │p       Center           │p     Panel      │
│ (Control)   │l       Panel            │l   (Properties) │
│             │i   (Visualization)      │i                │
│             │t                        │t                │
│             │t                        │t                │
├─────────────┼─────────────────────────┼─────────────────┤
│             │S                        │                 │
│             │p                        │                 │
│             │l                        │                 │
│             │i      Bottom Panel      │                 │
│             │t      (Status)          │                 │
│             │                         │                 │
│             │t                        │                 │
└─────────────┴─────────────────────────┴─────────────────┘

S = Splitter (resizable handle)
p = Splitter direction
l =
i =
t =
t =
e =
r =
```

### 2. Splitter Components
- **Invisible Button**: Acts as the draggable handle
- **Visual Feedback**: Mouse cursor changes and tooltips
- **Constraint System**: Minimum/maximum size enforcement
- **State Management**: Panel size persistence

### 3. Coordinate System
```cpp
// Panel layout calculation
float totalWidth = availableWidth;
float usedWidth = leftPanelWidth + rightPanelWidth + (splitterCount * splitterThickness);
float centerWidth = totalWidth - usedWidth;
```

## Implementation Architecture

### Class Structure

```cpp
class Gui {
private:
    // Panel size state (persistent)
    float currentLeftSidebarWidth = 320.0f;
    float currentRightSidebarWidth = 300.0f;
    float currentBottomBarHeight = 150.0f;

    // Default sizes (for reset functionality)
    float defaultLeftSidebarWidth = 320.0f;
    float defaultRightSidebarWidth = 300.0f;
    float defaultBottomBarHeight = 150.0f;

    // Panel visibility state
    bool showLeftSidebar = true;
    bool showRightSidebar = true;
    bool showBottomPanel = true;

public:
    void RenderMainInterface();
    void ResetPanelSizes();
    // ... other methods
};
```

### State Management Strategy

1. **Member Variables**: Store current panel sizes as class members
2. **Default Values**: Keep original sizes for reset functionality
3. **Visibility Flags**: Control which panels are shown
4. **Constraint Enforcement**: Maintain minimum/maximum sizes

## Step-by-Step Implementation

### Step 1: Basic Panel Layout Setup

```cpp
void Gui::RenderMainInterface() {
    // Get available space
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float availableHeight = ImGui::GetContentRegionAvail().y;

    // Define constraints
    const float minPanelWidth = 200.0f;
    const float minPanelHeight = 100.0f;
    const float splitterThickness = 4.0f;

    // Calculate used space
    float usedWidth = 0.0f;
    if (showLeftSidebar) usedWidth += currentLeftSidebarWidth + splitterThickness;
    if (showRightSidebar) usedWidth += currentRightSidebarWidth + splitterThickness;

    // Calculate remaining center space
    float centerWidth = availableWidth - usedWidth;
}
```

### Step 2: Constraint Enforcement

```cpp
// Ensure minimum sizes are respected
if (centerWidth < minPanelWidth) {
    float excess = minPanelWidth - centerWidth;

    // Redistribute excess by shrinking sidebars
    if (showLeftSidebar && showRightSidebar) {
        currentLeftSidebarWidth -= excess * 0.5f;
        currentRightSidebarWidth -= excess * 0.5f;
    } else if (showLeftSidebar) {
        currentLeftSidebarWidth -= excess;
    } else if (showRightSidebar) {
        currentRightSidebarWidth -= excess;
    }

    centerWidth = minPanelWidth;
}
```

### Step 3: Panel Rendering with Splitters

```cpp
// Left panel with resizable splitter
if (showLeftSidebar) {
    // Render the panel content
    ImGui::BeginChild("LeftPanel", ImVec2(currentLeftSidebarWidth, mainAreaHeight), true);
    RenderControlPanelContent();
    ImGui::EndChild();

    // Render the splitter
    ImGui::SameLine();
    RenderVerticalSplitter("##LeftSplitter", currentLeftSidebarWidth,
                          mainAreaHeight, availableWidth, minPanelWidth);
    ImGui::SameLine();
}
```

### Step 4: Splitter Implementation

```cpp
void RenderVerticalSplitter(const char* id, float& panelWidth, float height,
                           float maxWidth, float minWidth) {
    // Style the splitter button
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::GLASS_BORDER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::Colors::ACCENT_PRIMARY);

    // Create invisible button for dragging
    ImGui::Button(id, ImVec2(splitterThickness, height));

    // Handle dragging
    if (ImGui::IsItemActive()) {
        panelWidth += ImGui::GetIO().MouseDelta.x;
        panelWidth = ImClamp(panelWidth, minWidth, maxWidth);
    }

    // Visual feedback
    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        ImGui::SetTooltip("Drag to resize panel\nDouble-click to reset");
    }

    // Double-click to reset
    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
        panelWidth = defaultWidth; // Reset to default
    }

    ImGui::PopStyleColor(3);
}
```

### Step 5: Horizontal Splitter (Bottom Panel)

```cpp
// Bottom panel with horizontal splitter
if (showBottomPanel) {
    // Horizontal splitter
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::Colors::GLASS_BORDER);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::Colors::ACCENT_HOVER);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::Colors::ACCENT_PRIMARY);

    ImGui::Button("##BottomSplitter", ImVec2(-1, splitterThickness));

    if (ImGui::IsItemActive()) {
        currentBottomBarHeight -= ImGui::GetIO().MouseDelta.y; // Note: subtract for bottom panel
        currentBottomBarHeight = ImClamp(currentBottomBarHeight, minPanelHeight,
                                       availableHeight * 0.6f);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        ImGui::SetTooltip("Drag to resize Status Panel\nDouble-click to reset");
    }

    if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
        currentBottomBarHeight = defaultBottomBarHeight;
    }

    ImGui::PopStyleColor(3);

    // Render bottom panel
    ImGui::BeginChild("BottomPanel", ImVec2(-1, currentBottomBarHeight), true);
    RenderStatusPanelContent();
    ImGui::EndChild();
}
```

## Advanced Features

### 1. Reset Functionality

```cpp
void Gui::ResetPanelSizes() {
    // Reset all panels to default sizes
    currentLeftSidebarWidth = defaultLeftSidebarWidth;
    currentRightSidebarWidth = defaultRightSidebarWidth;
    currentBottomBarHeight = defaultBottomBarHeight;

    // Ensure all panels are visible
    showLeftSidebar = true;
    showRightSidebar = true;
    showBottomPanel = true;
}
```

### 2. Layout Persistence (Advanced)

```cpp
struct PanelLayout {
    float leftWidth;
    float rightWidth;
    float bottomHeight;
    bool leftVisible;
    bool rightVisible;
    bool bottomVisible;
};

void Gui::SavePanelLayout() {
    PanelLayout layout = {
        currentLeftSidebarWidth,
        currentRightSidebarWidth,
        currentBottomBarHeight,
        showLeftSidebar,
        showRightSidebar,
        showBottomPanel
    };

    // Save to file or registry
    // Implementation depends on your preferences
}

void Gui::LoadPanelLayout() {
    // Load from file or registry
    // Apply loaded values to current variables
}
```

### 3. Animated Resizing (Optional)

```cpp
class AnimatedPanel {
private:
    float targetWidth;
    float currentWidth;
    float animationSpeed = 5.0f;

public:
    void Update(float deltaTime) {
        if (abs(currentWidth - targetWidth) > 1.0f) {
            currentWidth = ImLerp(currentWidth, targetWidth,
                                 animationSpeed * deltaTime);
        }
    }

    void SetTarget(float width) { targetWidth = width; }
    float GetCurrent() const { return currentWidth; }
};
```

## Best Practices

### 1. User Experience
- **Visual Feedback**: Always change cursor and show tooltips
- **Minimum Sizes**: Prevent panels from becoming unusable
- **Double-Click Reset**: Provide easy way to restore defaults
- **Smooth Dragging**: Use appropriate mouse delta scaling

### 2. Code Organization
- **Member Variables**: Store panel sizes as class members, not static variables
- **Constraint Functions**: Separate constraint logic into helper functions
- **Consistent Naming**: Use clear, descriptive names for panel variables

### 3. Performance
- **Efficient Calculations**: Only recalculate when necessary
- **Minimal State Changes**: Avoid unnecessary updates
- **Proper Clamping**: Use ImClamp for efficient boundary checking

### 4. Maintainability
```cpp
// Good: Clear structure and separation of concerns
class PanelManager {
    struct PanelConfig {
        float width;
        float height;
        bool visible;
        float minSize;
        float maxSize;
    };

    PanelConfig leftPanel;
    PanelConfig rightPanel;
    PanelConfig bottomPanel;

public:
    void RenderPanelWithSplitter(PanelConfig& config, /* ... */);
    void ResetPanel(PanelConfig& config);
    void SaveLayout();
    void LoadLayout();
};
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Panels Disappearing
**Problem**: Panels become too small and disappear
**Solution**: Implement proper minimum size constraints
```cpp
// Always enforce minimum sizes
panelWidth = ImClamp(panelWidth, MIN_PANEL_WIDTH, maxAllowedWidth);
```

#### 2. Jerky Resizing
**Problem**: Splitters jump or behave erratically
**Solution**: Ensure proper mouse delta handling
```cpp
// Use mouse delta, not absolute position
if (ImGui::IsItemActive()) {
    panelWidth += ImGui::GetIO().MouseDelta.x; // Smooth delta-based resizing
}
```

#### 3. Overlapping Panels
**Problem**: Panels overlap when window is resized
**Solution**: Implement responsive constraint system
```cpp
// Recalculate constraints every frame
float totalRequired = leftWidth + rightWidth + centerMinWidth + splitterWidths;
if (totalRequired > availableWidth) {
    // Redistribute space proportionally
}
```

#### 4. State Not Persisting
**Problem**: Panel sizes reset on restart
**Solution**: Use member variables, not static variables
```cpp
// Good: Member variables persist with object
class Gui {
    float currentLeftSidebarWidth = 320.0f; // Member variable
};

// Bad: Static variables can be problematic
void SomeFunction() {
    static float width = 320.0f; // Hard to reset externally
}
```

## Performance Considerations

### 1. Calculation Optimization
```cpp
// Cache expensive calculations
class PanelLayout {
private:
    mutable bool layoutDirty = true;
    mutable float cachedCenterWidth;

public:
    float GetCenterWidth() const {
        if (layoutDirty) {
            cachedCenterWidth = CalculateCenterWidth();
            layoutDirty = false;
        }
        return cachedCenterWidth;
    }

    void MarkDirty() { layoutDirty = true; }
};
```

### 2. Rendering Efficiency
- Only render visible panels
- Use ImGui::IsItemVisible() for conditional rendering
- Minimize style push/pop operations

### 3. Memory Management
- Avoid dynamic allocations in render loop
- Use fixed-size arrays for panel configurations
- Pre-allocate tooltip strings

## Example Integration

Here's a complete minimal example:

```cpp
class ResizablePanelDemo {
private:
    float leftWidth = 200.0f;
    float rightWidth = 200.0f;
    float bottomHeight = 100.0f;

    const float SPLITTER_SIZE = 4.0f;
    const float MIN_PANEL_SIZE = 100.0f;

public:
    void Render() {
        float availWidth = ImGui::GetContentRegionAvail().x;
        float availHeight = ImGui::GetContentRegionAvail().y;

        float centerWidth = availWidth - leftWidth - rightWidth - 2 * SPLITTER_SIZE;
        float mainHeight = availHeight - bottomHeight - SPLITTER_SIZE;

        // Left panel
        ImGui::BeginChild("Left", ImVec2(leftWidth, mainHeight), true);
        ImGui::Text("Left Panel Content");
        ImGui::EndChild();

        // Left splitter
        ImGui::SameLine();
        ImGui::Button("##LSplit", ImVec2(SPLITTER_SIZE, mainHeight));
        if (ImGui::IsItemActive()) {
            leftWidth += ImGui::GetIO().MouseDelta.x;
            leftWidth = ImClamp(leftWidth, MIN_PANEL_SIZE, availWidth - 300.0f);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        // Center panel
        ImGui::SameLine();
        ImGui::BeginChild("Center", ImVec2(centerWidth, mainHeight), true);
        ImGui::Text("Center Panel Content");
        ImGui::EndChild();

        // Right splitter and panel (similar implementation)
        // Bottom splitter and panel (similar implementation)
    }
};
```

This comprehensive guide provides all the necessary information to implement professional resizable panels in ImGui applications. The key is to maintain clean state management, provide proper user feedback, and ensure responsive behavior across different window sizes.
